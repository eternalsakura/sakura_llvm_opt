//
// Created by sakura on 2020/7/12.
//

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
            // OUT[entry] = Ø
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
            bool hasChanges = false;
            BitVector temp_obv = ibv;
            // use
            for (const Use &op : inst.operands()) {
                //如果变量在domain中
                // op的类型是use，因为Use中定义了operator Value *() const { return Val; }
                // 这个将Use转换为其他类型值的类型转换规则，所以我们在需要Value*的地方可以直接传入op
                int use_idx = position(Variable(op));
                if (use_idx != -1) {
                    temp_obv[use_idx] = true;
                }
            }
            // def
            int def_idx = position(Variable(&inst));
            if (def_idx != -1) {
                temp_obv[def_idx] = false;
            }
            //如果和obv不相等
            hasChanges = temp_obv != obv;
            obv = temp_obv;
            return hasChanges;
        }

        virtual void InitializeDomainFromInstruction(const Instruction &inst) override {
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

        virtual ~Liveness() override {}

    };

    char Liveness::ID = 1;
    RegisterPass<Liveness> Y("liveness", "Liveness");

} // namespace anonymous