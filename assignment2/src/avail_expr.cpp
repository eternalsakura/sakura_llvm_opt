//
// Created by sakura on 2020/7/13.
//
#include "framework.h"

// 初始状态, 注意_inst_bv_map总是保存着"经过传递函数变换之后的state",即对于前向数据流分析，
// 它保存的是output state，而对于后向数据流分析，则保存的是input state。

// 一个程序只能有entry一个入口点
// entry:
    // input1 = BC = ∅
    // instr1 :
    // output1 = IC = U

    // input2 =
    // instr2 :
    // output2 = IC = U

// bb1:
//  ...

// bb2:
//假设bb2的前驱是entry和bb1,则
    //input1 =  output[bb1] ∩ output[entry]
    //instr1:
    //output1 = IC = U
    // input2 =
    // instr2 :
    // output2 = IC = U

namespace {

    class Expression {
    private:
        unsigned _opcode;
        const Value *_lhs, *_rhs;
    public:
        Expression(const Instruction &inst) {
            _opcode = inst.getOpcode();
            _lhs = inst.getOperand(0);
            _rhs = inst.getOperand(1);
        }

        bool operator==(const Expression &Expr) const {
            bool isEquOperand = false;
            switch (_opcode) {
                case Instruction::Add:
                case Instruction::FAdd:
                case Instruction::Mul:
                case Instruction::FMul:
                case Instruction::And:
                case Instruction::Or:
                case Instruction::Xor:
                    isEquOperand = (Expr.getLHSOperand() == _lhs && Expr.getRHSOperand() == _rhs)
                                   || (Expr.getLHSOperand() == _rhs && Expr.getRHSOperand() == _lhs);
                    break;
                default:
                    isEquOperand = Expr.getLHSOperand() == _lhs && Expr.getRHSOperand() == _rhs;
            }
            return Expr.getOpcode() == _opcode && isEquOperand;
        }


        unsigned getOpcode() const { return _opcode; }

        const Value *getLHSOperand() const { return _lhs; }

        const Value *getRHSOperand() const { return _rhs; }

        friend raw_ostream &operator<<(raw_ostream &outs, const Expression &expr);
    };

    raw_ostream &operator<<(raw_ostream &outs, const Expression &expr) {
        outs << "[" << Instruction::getOpcodeName(expr._opcode) << " ";
        expr._lhs->printAsOperand(outs, false);
        outs << ", ";
        expr._rhs->printAsOperand(outs, false);
        outs << "]";

        return outs;
    }

}  // namespace anonymous

namespace std {

// Construct a hash code for 'Expression'.
    template<>
    struct hash<Expression> {
        std::size_t operator()(const Expression &expr) const {
            std::hash<unsigned> unsigned_hasher;
            std::hash<const Value *> pvalue_hasher;

            std::size_t opcode_hash = unsigned_hasher(expr.getOpcode());
            std::size_t lhs_operand_hash = pvalue_hasher((expr.getLHSOperand()));
            std::size_t rhs_operand_hash = pvalue_hasher((expr.getRHSOperand()));

            return opcode_hash ^ (lhs_operand_hash << 1) ^ (rhs_operand_hash << 1);
        }
    };

}  // namespace std


namespace {

    class AvailExpr final : public dfa::Framework<Expression,
            dfa::Direction::Forward> {
    protected:
        virtual BitVector IC() const override {
            return BitVector(_domain.size(), true);
        }

        virtual BitVector BC() const override {
            return BitVector(_domain.size(), false);
        }

        virtual BitVector MeetOp(const meetop_const_range &meet_operands) const override {
            BitVector result(_domain.size(), true);
            for (const BasicBlock *bb:meet_operands) {
                result &= _inst_bv_map.at(&bb->back());
            }
            return result;
        }

        virtual BitVector MeetOp(const BasicBlock &bb) const override {
            // 仅实现，不使用
            BitVector result(_domain.size(), true);
            return result;
        }

        virtual bool TransferFunc(const Instruction &inst,
                                  const BitVector &ibv,
                                  BitVector &obv) override {
            bool hasChanges = false;
            //  f(x) = e_genB ∪ (x - e_killB)
            BitVector temp_obv = ibv;

            // gen, 首先判断是否inst是二元运算，然后查找当前指令保存的表达式是否在domain中
            if (isa<BinaryOperator>(inst) && _domain.find(Expression(inst)) != _domain.end())
                temp_obv[position(Expression(inst))] = true;

            // kill
            for (const Expression &elem : _domain) {
                if (elem.getLHSOperand() == &inst || elem.getRHSOperand() == &inst) {
                    temp_obv[position(elem)] = false;
                }
            }

            bool changed = temp_obv != obv;
            obv = temp_obv;
            return changed;
        }

        virtual void InitializeDomainFromInstruction(const Instruction &inst) override {
            // 将所有二元运算的inst插入_domain中
            if (isa<BinaryOperator>(inst)) {
                _domain.emplace(inst);
            }
        }

    public:
        static char ID;

        AvailExpr() : dfa::Framework<domain_element_t,
                direction_c>(ID) {}

        virtual ~AvailExpr() override {}
    };

    char AvailExpr::ID = 1;
    RegisterPass<AvailExpr> Y("avail_expr", "Available Expression");

} // namespace anonymous

