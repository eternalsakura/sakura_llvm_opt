//
// Created by sakura on 2020/7/13.
//

#ifndef ASSIGNMENT2_FRAMEWORK_H
#define ASSIGNMENT2_FRAMEWORK_H

#include <cassert>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include <llvm/Pass.h>
#include <llvm/ADT/BitVector.h>
#include <llvm/ADT/PostOrderIterator.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>
#include "analysis_flag.h"
using namespace llvm;
namespace dfa {
    //analysis direction, 用作模板参数
    enum class Direction {
        Forward, Backward
    };

    /// Dataflow Analysis Framework
    ///
    /// @tparam TDomainElement Domain Element
    /// @tparam TDirection     Direction of Analysis
    template<class TDomainElement, Direction TDirection>
    class Framework : public FunctionPass {

        //    enable_if
        //    若B为true，则std::enable_if拥有等同于T的public成员typedef type；
        //    否则，无该成员typedef

        //    enable_if 可能的实现
        //    template<bool B, class T = void>
        //    struct enable_if {};
        //
        //    template<class T>
        //    struct enable_if<true, T> { typedef T type; };

/// @brief 这个macro根据analysis direction有选择的启用方法
/// @param dir Direction of Analysis
/// @pram ret Return Type
/// @comment 返回ret指定的类型
#define METHOD_ENABLE_IF_DIRECTION(dir, ret)    \
    template <Direction _TDirection = TDirection>   \
    typename std::enable_if<_TDirection == dir, ret>::type

/// @brief 这个macro根据analysis direction进行typedef的操作,前向则定义TrueTy, 后向则定义FalseTy.
/// @param type_name  Name of the Resulting Type
/// @param dir        Direction of Analysis
/// @param T          Type Definition if `TDirection == dir`
#define TYPEDEF_IF_DIRECTION(type_name, dir, TrueTy, FalseTy)                                 \
        using type_name = typename std::conditional<dir == TDirection, TrueTy, FalseTy>::type;
    protected:
        typedef TDomainElement domain_element_t;
        static constexpr Direction direction_c = TDirection;
        /***********************************************************************
        * Domain
        ***********************************************************************/
        std::unordered_set<TDomainElement> _domain;
        /***********************************************************************
         * Instruction-BitVector Mapping
         ***********************************************************************/
        //建立instruction pointer到BitVector的map
        std::unordered_map<const Instruction *, BitVector> _inst_bv_map;

        /// @biref 返回初始条件
        /// @todo 在子类覆盖这个方法
        virtual BitVector IC() const = 0;

        /// @biref 返回边界条件
        /// @todo 在子类覆盖这个方法
        virtual BitVector BC() const = 0;

    private:
        /// @biref
        /// Dump the domain under @p mask .
        /// If @c domain = {%1, %2, %3,}, dumping it with @p mask = 001 will give {%3,}
        void printDomainWithMask(const BitVector &mask) const {
            outs() << "{";
            assert(mask.size() == _domain.size() && "The size of mask must be equal to the size of domain.");
            unsigned mask_idx = 0;
            // 若代表domain对应节点的mask bit不为0，则打印，否则跳过。
            for (const auto &elem : _domain) {
                assert(mask_idx >= 0 && mask_idx < mask.size());
                if (!mask[mask_idx++]) {
                    continue;
                }
                outs() << elem << ",";
            }// for (mask_idx ∈ [0, mask.size()))
            outs() << "}";
        }

        METHOD_ENABLE_IF_DIRECTION(Direction::Forward, void)
        printInstBV(const Instruction &inst) const {
            const BasicBlock *const pbb = inst.getParent();
            if (&inst == &(*pbb->begin())) {
                meetop_const_range meet_operands = MeetOperands(*pbb);
                // 如果前驱节点为空，则我们位于边界，打印出BC
                if (meet_operands.empty()) {
                    outs() << "BC:\t";
                    printDomainWithMask(BC());
                    outs() << "\n";
                } else {
                    outs() << "MeetOp:\t";
                    printDomainWithMask(MeetOp(meet_operands));
                    outs() << "\n";
                }
            }
            outs() << "Instruction: " << inst << "\n";
            outs() << "\t";
            printDomainWithMask(_inst_bv_map.at(&inst));
            outs() << "\n";
        }

        METHOD_ENABLE_IF_DIRECTION(Direction::Backward, void)
        printInstBV(const Instruction &inst) const {
            const BasicBlock *const pbb = inst.getParent();

            if (&inst == &(*pbb->begin())) {
                meetop_const_range meet_operands = MeetOperands(*pbb);
                //如果后继节点为空，则我们位于边界，打印出BC
                if (meet_operands.empty()) {
                    outs() << "BC:\t";
                    printDomainWithMask(BC());
                    outs() << "\n";
                } else {
                    outs() << "MeetOp:\t";
#ifndef LA
                    printDomainWithMask(MeetOp(meet_operands));
#else
                    printDomainWithMask(MeetOp(*pbb));
#endif
                    outs() << "\n";
                }
            }
            outs() << "Instruction: " << inst << "\n";
            outs() << "\t";
            printDomainWithMask(_inst_bv_map.at(&inst));
            outs() << "\n";
        }

    protected:
        /// @brief Dump, ∀inst ∈ @p F, the associated bitvector.
        void printInstBVMap(const Function &F) const {
            outs() << "********************************************" << "\n";
            outs() << "* Instruction-BitVector Mapping             " << "\n";
            outs() << "********************************************" << "\n";

            for (const auto &inst : instructions(F)) {
                printInstBV(inst);
            }
        }
        /***********************************************************************
         * Meet Operator and Transfer Function
         ***********************************************************************/
        /// @brief 如果是前向分析，则meetop_const_range定义为pred，否则为succ。
        TYPEDEF_IF_DIRECTION(meetop_const_range, Direction::Forward, pred_const_range, succ_const_range);

        /// @brief 返回一个可以遍历所有前向节点的迭代器range, 即返回Meet operation的操作数(operands)
        METHOD_ENABLE_IF_DIRECTION(Direction::Forward, meetop_const_range)
        MeetOperands(const BasicBlock &bb) const {
            return predecessors(&bb);
        }

        /// @brief 返回一个可以遍历所有后继节点的迭代器range
        METHOD_ENABLE_IF_DIRECTION(Direction::Backward, meetop_const_range)
        MeetOperands(const BasicBlock &bb) const {
            return successors(&bb);
        }

        /// @brief  在所有meet_operands上执行meet operation.
        /// @return 执行完meet operation之后的result bitVector.
        /// @todo   在每个子类的方法里覆盖这个方法
        virtual BitVector MeetOp(const meetop_const_range &meet_operands) const = 0;

        /// @brief  在所有meet_operands上执行meet operation.
        /// @return 执行完meet operation之后的result bitVector.
        /// @todo   在每个子类的方法里覆盖这个方法
        virtual BitVector MeetOp(const BasicBlock &bb) const = 0;


        /// @brief  将指令@p inst的传递函数应用于传入的input bitvector，以获取output bitvector
        /// @return 如果@p obv改变则返回true, 否则返回false.
        /// @todo   在每个子类的方法里覆盖这个方法

        virtual bool TransferFunc(const Instruction &inst,
                                  const BitVector &ibv,
                                  BitVector &obv) = 0;

        /***********************************************************************
         * CFG Traversal
         ***********************************************************************/
    private:
        /// @brief 如果@p dir是Forward，则bb_traversal_const_range为const_iterator range，否则为const_reverse_iterator range
        TYPEDEF_IF_DIRECTION(bb_traversal_const_range, Direction::Forward,
                             iterator_range<Function::const_iterator>,
                             iterator_range<Function::BasicBlockListType::const_reverse_iterator>);

        /// @brief 如果@p dir是Forward，则inst_traversal_const_range为const_iterator range，否则为const_reverse_iterator range
        TYPEDEF_IF_DIRECTION(inst_traversal_const_range, Direction::Forward,
                             iterator_range<BasicBlock::const_iterator>,
                             iterator_range<BasicBlock::InstListType::const_reverse_iterator>);


        /// @brief Return the traversal order of the basic blocks.
        METHOD_ENABLE_IF_DIRECTION(Direction::Forward, bb_traversal_const_range)
        BBTraversalOrder(const Function &F) const {
            return make_range(F.begin(), F.end());
        }

        METHOD_ENABLE_IF_DIRECTION(Direction::Backward, bb_traversal_const_range)
        BBTraversalOrder(const Function &F) const {
            return make_range(F.getBasicBlockList().rbegin(), F.getBasicBlockList().rend());
        }

        /// @brief Return the traversal order of the instructions.
        METHOD_ENABLE_IF_DIRECTION(Direction::Forward, inst_traversal_const_range)
        InstTraversalOrder(const BasicBlock &bb) const {
            return make_range(bb.begin(), bb.end());
        }

        METHOD_ENABLE_IF_DIRECTION(Direction::Backward, inst_traversal_const_range)
        InstTraversalOrder(const BasicBlock &bb) const {
            return make_range(bb.getInstList().rbegin(), bb.getInstList().rend());
        }

    protected:
        /// @brief 找到_domain中的elem对应在bitvector的位置
        int position(const TDomainElement &elem) const {
            auto elem_iter = _domain.find(elem);
            if (elem_iter == _domain.end()) {
//                assert(elem_iter == _domain.end());
                return -1;
            } else {
                int pos = 0;
                //顺序遍历domain，以找到elem在domain中对应的位置
                for (auto domain_iter = _domain.begin(); domain_iter != elem_iter; domain_iter++, pos++) {}
                return pos;
            }
        }

        /// @brief  遍历CFG图并更新@c inst_bv_map .
        /// @return 如果@c inst_bv_map被改变则返回true, 否则返回false
        /// @todo   实现这个方法为所有子类
        bool traverseCFG(const Function &func) {
            bool changed = false;
            //获取entry basic block的地址
            const BasicBlock *entry = &*(BBTraversalOrder(func).begin());
            BitVector initialBV, inputBV;
            for (auto &bb : BBTraversalOrder(func)) {
                if (entry == &bb) {
                    // initialBV <- Boundary Condition
                    initialBV = BC();
                } else {
                    // initialB <- MeetOp(MeetOperands(bb));
#ifndef LA
                    initialBV = MeetOp(MeetOperands(bb));
#else
                    initialBV = MeetOp(bb);
#endif
                }
                inputBV = initialBV;
                for (auto &inst : InstTraversalOrder(bb)) {
                    changed |= TransferFunc(inst, inputBV, _inst_bv_map.at(&inst));
                    //顺着数据流分析的方向，上一条指令的"output"即下一条指令的"input"
                    inputBV = _inst_bv_map.at(&inst);
                }
            }
            return changed;
        }

    public:
        Framework(char ID) : FunctionPass(ID) {}

        virtual ~Framework() override {}

        // We don't modify the program, so we preserve all analysis.
        virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
            AU.setPreservesAll();
        }

    protected:
        /// @brief 依据每条inst来初始化domain
        /// @todo  Override this method in every child class.
        virtual void InitializeDomainFromInstruction(const Instruction &inst) = 0;

    public:
        virtual bool runOnFunction(Function &F) override final {
            //遍历每条指令，初始化domain
            for (const auto &inst : instructions(F)) {
                InitializeDomainFromInstruction(inst);
            }
            //向_inst_bv_map依次添加初始状态的instruction-bv键值对
            for (const auto &inst : instructions(F)) {
                _inst_bv_map.emplace(&inst, IC());
            }
            // 不断遍历CFG,直到instruction-bv不发生变化
            while (traverseCFG(F)) {}
            // dump结果
            printInstBVMap(F);
            return false;
        }

#undef METHOD_ENABLE_IF_DIRECTION
    };
}
#endif //ASSIGNMENT2_FRAMEWORK_H