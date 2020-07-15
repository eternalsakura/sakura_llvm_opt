//
// Created by sakura on 2020/7/12.
//

// 初始状态, 注意_inst_bv_map总是保存着"经过传递函数变换之后的state",即对于前向数据流分析，
// 它保存的是output state，而对于后向数据流分析，则保存的是input state。
// 在同一个基本块内，顺着数据流分析的方向，上一条指令的"output"即下一条指令的"input"
// 另外要记得，我们初始化的，永远是经过传递函数变化之后的那个真正的"output",对于前向就是output，对于后向就是input

// 下面按照后向去画图
// 一个程序只能有"entry（其实叫end贴切一点)"一个出口点

// end:
    // outputN = BC = ∅
    // instrN :
    // inputN = IC = ∅
    // ...
    // output1 =
    // instr1 :
    // input1 = IC = ∅

// bb2:
//  ...

// bb1:
//假设bb1的后继是entry和bb2,则
    // outputN = input[bb2] ∪ input[entry]
    // instrN:
    // inputN = IC = ∅

    // output1 =
    // instr1 :
    // input1 = IC = ∅

#include "framework.h"

using namespace llvm;

namespace {
    class Variable {
    private:
        const Value *_value;
    public:
        explicit Variable(const Value *value) : _value(value) {}

        bool operator==(const Variable &variable) const {
            return _value == variable._value;
        }

        const Value *getValue() const {
            return _value;
        }

        friend raw_ostream &operator<<(raw_ostream &outs, const Variable &var);
    };

    raw_ostream &operator<<(raw_ostream &outs, const Variable &var) {
        outs << "[";
        var._value->printAsOperand(outs, false);
        outs << "]";
        return outs;
    }
}// namespace anonymous

namespace std {

// Construct a hash code for 'Expression'.
    template<>
    struct hash<Variable> {
        std::size_t operator()(const Variable &var) const {
            std::hash<const Value *> pvalue_hasher;

            std::size_t value_hash = pvalue_hasher((var.getValue()));

            return value_hash;
        }
    };

}  // namespace std

namespace {
    class Liveness final : public dfa::Framework<Variable, dfa::Direction::Backward> {
    protected:
        virtual BitVector IC() const override {
            return BitVector(_domain.size());
        }

        virtual BitVector BC() const override {
            return BitVector(_domain.size());
        }

        // 仅实现，不使用
        virtual BitVector MeetOp(const meetop_const_range &meet_operands) const override {
            // U In[S]
            BitVector result(_domain.size(), false);
            return result;
        }

        virtual BitVector MeetOp(const BasicBlock &bb) const override {
            BitVector result(_domain.size(), false);
            for (const BasicBlock *succ_bb_ptr : successors(&bb)) {
                //所有后继基本块的第一条Instr的IN集合的并集，就是当前基本块的OUT集
                BitVector succ_bv = _inst_bv_map.at(&succ_bb_ptr->front());
                // 对含有phi指令的基础块作特殊处理，因为phi指令涉及到的变量需要来自于我们要处理的块才是活跃的
                // 当前处理的后继块遍历所有phi指令
                for (const PHINode &phi : succ_bb_ptr->phis()) {
                    //遍历当前后继基本块里的phi指令所有可能的前驱基本块
                    for (const BasicBlock *phi_pred_bb_ptr : phi.blocks()) {
                        //如果当前前驱基本块不是现在的基本块
                        if (phi_pred_bb_ptr != &bb) {
                            //得到该基本块中定义的变量
                            const Value *val = phi.getIncomingValueForBlock(phi_pred_bb_ptr);
                            //如果该变量在domain中存在，将该变量在IN集合中的状态设置为false
                            int idx = position(Variable(val));
                            if (idx != -1) {
                                // 将临时变量中对应变量的bit设置为false
                                assert(succ_bv[idx] = true);
                                succ_bv[idx] = false;
                            }
                        }
                    }
                }
                result |= succ_bv;
            }
            return result;
        }

        virtual bool TransferFunc(const Instruction &inst,
                                  const BitVector &ibv,
                                  BitVector &obv) override {
            // use U (In - def)
            bool hasChanges;
            BitVector temp_obv = ibv;
//            // use
            for (auto &op : inst.operands()) {
//                //如果变量在domain中
                const Value *op_val = dyn_cast<Value>(op.get());
                assert(op_val != NULL);
                int use_idx = position(Variable(op_val));
                if (use_idx != -1) {
                    temp_obv[use_idx] = true;
                }
            }
//            // def
            const Value *inst_op = dyn_cast<Value>(&inst);
            assert(inst_op != NULL);
            int def_idx = position(Variable(inst_op));
            if (def_idx != -1) {
                temp_obv[def_idx] = false;
            }
            //如果和obv不相等
            hasChanges = temp_obv != obv;
            obv = temp_obv;
            return hasChanges;
        }


        virtual void InitializeDomainFromInstruction(const Instruction &inst)
        override {
            for (const Use &op : inst.operands()) {
                if (isa<Instruction>(op) || isa<Argument>(op)) {
                    _domain.emplace(Variable(op));
                }
            }
        }

    public:
        static char ID;

        Liveness() : dfa::Framework<domain_element_t,
                direction_c>(ID) {}

        virtual ~Liveness()
        override {}

    };

    char Liveness::ID = 1;
    RegisterPass<Liveness> Y(
            "liveness", "Liveness");

} // namespace anonymous