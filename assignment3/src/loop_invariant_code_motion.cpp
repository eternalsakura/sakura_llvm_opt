//
// Created by sakura on 2020/7/16.
//

#include <llvm/Analysis/LoopPass.h>
#include <llvm/Analysis/ValueTracking.h>
#include <llvm/IR/Dominators.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

#define FOUND(list, elem) \
    std::find(list.begin(), list.end(), elem) == list.end()

namespace {

    class LoopInvariantCodeMotion final : public LoopPass {
    private:
        DominatorTree *dom_tree;  // owned by `DominatorTreeWrapperPass`
        std::list<Instruction *> MarkedAsInvariant;
    public:
        static char ID;

        LoopInvariantCodeMotion() : LoopPass(ID) {}

        virtual ~LoopInvariantCodeMotion() override {}

        virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
            AU.addRequired<DominatorTreeWrapperPass>();
            AU.addRequired<LoopInfoWrapperPass>();
            AU.setPreservesCFG();
        }

        /// @todo Finish the implementation of this method.
        virtual bool runOnLoop(Loop *L, LPPassManager &LPM) override {
            dom_tree = &(getAnalysis<DominatorTreeWrapperPass>().getDomTree());
            outs() << "ENTRY ################################################\n";
            // 如果前置首结点不存在，那就不优化
            if (!L->getLoopPreheader())
                return false;
            // 从DominatorTreeWrapperPass分析获取DomTree
            dom_tree = &(getAnalysis<DominatorTreeWrapperPass>().getDomTree());
            // 测试了一下，发现该Pass内的变量存活至下一个循环，所以必须清空。
            MarkedAsInvariant.clear();
            bool IrHasChanged = false;
            bool HasChanged;
            do {
                HasChanged = false;
                // 遍历基本块
                for (BasicBlock *BB: L->blocks()) {
                    LoopInfo *LoopI = &(getAnalysis<LoopInfoWrapperPass>().getLoopInfo());
                    // 可能会遇到嵌套循环的情况。此时不对嵌套循环的指令做处理,
                    // 留给下一次嵌套循环调用当前pass了再处理
                    // 之所以判断处用了LoopI->getLoopFor而不是LoopI->contain，
                    // 是因为一个是自下而上，另一个是自上而下
                    // getLoopFor可以返回基本块内最内层的循环
                    if (LoopI->getLoopFor(BB) == L) {
                        for (Instruction &Inst : BB->getInstList()) {
                            // 如果先前该指令没有被标记，且是循环不变量
                            // 则将其标记为循环不变量，并继续下一轮循环，因为可能会有依赖于这个循环不变量的新的循环不变量被发现。
                            if (FOUND(MarkedAsInvariant, &Inst)
                                && isInvariant(&Inst, L)) {
                                HasChanged = true;
                                MarkedAsInvariant.push_back(&Inst);
                            }
                        }
                    }
                }
            } while (HasChanged);
            // 成功移动的指令个数
            int moveCount = 0;
            // 注意，要按照指令的顺序来判断与移动指令
            // 当初用std::list就是看重它的有序性
            for (Instruction *Inst : MarkedAsInvariant) {
                /*
                    移动代码的三个条件：
                        1. s所在的基本块是循环所有出口结点(有后继结点在循环外的结点)的支配结点
                        2. 循环中没有其它语句对x赋值
                        3. 循环中对语句s:x=y+z中，x的引用仅由s到达
                */
                if (isDomExitBlocks(Inst, L) && AssignOnce(Inst) && OneWayToReferences(Inst)) {
                    // 将指令移动到循环的前置首节点
                    moveToPreheader(Inst, L);
                    outs() << "移出循环的指令 " << moveCount << " : " << *Inst << "\n";
                    IrHasChanged = true;
                    moveCount++;
                }
            }

            outs() << "循环不变量数量：\t\t" << MarkedAsInvariant.size() << "\n";
            outs() << "已移出循环的不变量数量：\t" << moveCount << "\n";
            outs() << "EXIT #################################################\n\n";
            return IrHasChanged;
        }

        // 检查指令inst所在的基本块是否是循环所有exit节点的支配节点
        bool isDomExitBlocks(Instruction *Inst, Loop *L) {
            SmallVector<BasicBlock *, 0> exitBlock;
            L->getExitBlocks(exitBlock);
            for (BasicBlock *BB : exitBlock) {
                // 只要有一个basicblock没有被inst所在的基础块所支配，那就返回false
                if (!dom_tree->dominates(Inst->getParent(), BB)) {
                    return false;
                }
            }
            return true;
        }

        // 检查循环中是否没有其它语句对x赋值
        bool AssignOnce(Instruction *inst) {
            // 由于SSA的特殊性，所有IR指令中，不存在其他语句会对某个"IR变量"进行不同位置的赋值
            return true;
        }

        // 检查循环中对x的引用是否仅由inst到达
        bool OneWayToReferences(Instruction *inst) {
            // 由于SSA的特殊性，所有IR指令中，只有一条语句会引用某个"IR变量"结果。
            return true;
        }

        // 将指令移动到前置首节点的末尾
        bool moveToPreheader(Instruction *Inst, Loop *loop) {
            BasicBlock *Preheader = loop->getLoopPreheader();
            // 都执行到这里了，当前循环的前置首节点不可能为空
            assert(Preheader != NULL);
            /*
                注意函数使用的是move，该函数会自动断开与原来基础块的连接，并移动到新的指令后面
                注意是移动到preheader的终结指令 *之前* 而不是 *之后* ，原因是
                    1. 循环前置首节点最后一条指令是终结指令，移动到终结指令之后则肯定无法执行
                    2. 最关键的一点，会报错，提示没有terminator
                基本块的终结指令基本上就是跳转指令
                同时，在这种情况下，不存在说，把指令移出来了，而该指令的操作数还在循环内部定值
            */
            Inst->moveBefore(Preheader->getTerminator());
            return false;
        }


        // 检查当前指令是否是循环不变量
        bool isInvariant(Instruction *Inst, Loop *L) {
            bool IsInvariants = true;
            for (Value *op : Inst->operands()) {
                // 如果当前操作数既不是一个常量，也不是函数参数
                if (!isa<Constant>(op) && !isa<Argument>(op)) {
                    if (Instruction *OpInst = dyn_cast<Instruction>(op)) {
                        // 且该操作数不是循环不变量，且该指令在循环内被定值
                        // getParent可以拿到该操作数被定值的基本块，如果它不在Loop里，那显然就不是在循环里被定值的
                        // 则传入的指令不是循环不变量
                        if (FOUND(MarkedAsInvariant, OpInst) && L->contains(OpInst->getParent())) {
                            IsInvariants = false;
                        } else {
                            IsInvariants = false;
                        }
                    }
                }
            }
            return isSafeToSpeculativelyExecute(Inst)  // 检查未定义错误，例如除以0。
                   // 这一条指令可以帮助过滤PHI和跳转指令
                   && !Inst->mayReadFromMemory()      // 修改读取内存的指令，可能会导致结果值的改变，所以不予处理
                   && !isa<LandingPadInst>(Inst)      // 异常处理相关的指令，必须在循环内部
                   && IsInvariants;
        }
    };

    char LoopInvariantCodeMotion::ID = 0;

    RegisterPass<LoopInvariantCodeMotion> X(
            "loop-invariant-code-motion",
            "Loop Invariant Code Motion");

}  // namespace anonymous