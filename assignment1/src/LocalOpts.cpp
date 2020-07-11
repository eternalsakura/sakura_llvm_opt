//
// Created by sakura on 2020/7/9.
//

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include <iostream>

#define DEBUG 1
#define AL 1
#define CF 1
#define ST 1

using namespace llvm;

namespace {
    class LocalOpts : public FunctionPass {
    public:
        static char ID;

        LocalOpts() : FunctionPass(ID) {};

        virtual ~LocalOpts() override {}

        int AlgebraicOptNum;
        int ConstantFoldOptNum;
        int StrengthOptNum;

        virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
            AU.setPreservesAll();
        }

        // Do some initialization
        virtual bool doInitialization(Module &) override {
            outs() << "LocalOpts" << '\n';
            AlgebraicOptNum = 0;
            ConstantFoldOptNum = 0;
            StrengthOptNum = 0;
            return false;
        }

        virtual bool runOnFunction(Function &F) override {
            if (CF)
                constantFold(F);
            if (AL)
                algebraic(F);
            if (ST)
                strength(F);

            outs() << "Transformations applied:" << "\n";
            outs() << "  Algebraic identities: " << AlgebraicOptNum << "\n";
            outs() << "  Constant folding: " << ConstantFoldOptNum << "\n";
            outs() << "  Strength reduction: " << StrengthOptNum << "\n";

            return false;
        }

        void deleteDeadInstrs(std::vector<Instruction *> instrs) {
            for (auto &instr:instrs) {
                if (instr->isSafeToRemove()) {
                    instr->eraseFromParent();
                }
            }
        }

        int getShift(int64_t x) {
            // http://www.exploringbinary.com/
            // ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/
            // if not power of two, return
            if (x <= 0 || (x & (~x + 1)) != x) {
                return -1;
            }
            // if power of two, get the number of shifts necessary
            int i = 0;
            while (x > 1) {
                x >>= 1;
                ++i;
            }
            return i;
        }

        void algebraic(Function &F) {
            std::vector<Instruction *> DeadInstrList;
            for (auto &BB : F) {
                for (auto &Instr: BB) {
                    if (Instr.getNumOperands() == 2) {
                        Value *Opd1 = Instr.getOperand(0);
                        Value *Opd2 = Instr.getOperand(1);
                        int64_t ConstVal1, ConstVal2;
                        if (isa<ConstantInt>(Opd1)) {
                            ConstVal1 = dyn_cast<ConstantInt>(Opd1)->getSExtValue();
                        }
                        if (isa<ConstantInt>(Opd2)) {
                            ConstVal2 = dyn_cast<ConstantInt>(Opd2)->getSExtValue();
                        }
                        bool AlgebraicFlag = true;
                        switch (Instr.getOpcode()) {
                            case Instruction::Add:
                                if (isa<ConstantInt>(Opd1) && ConstVal1 == 0) {
                                    // 0 + x
                                    Instr.replaceAllUsesWith(Opd2);
                                } else if (isa<ConstantInt>(Opd2) && ConstVal2 == 0) {
                                    // x + 0
                                    Instr.replaceAllUsesWith(Opd1);
                                } else {
                                    AlgebraicFlag = false;
                                }
                                break;
                            case Instruction::Sub:
                                if (isa<ConstantInt>(Opd2) && ConstVal2 == 0) {
                                    // x - 0
                                    Instr.replaceAllUsesWith(Opd2);
                                } else if (Opd1 == Opd2) {
                                    // x - x
                                    Instr.replaceAllUsesWith(ConstantInt::getSigned(Instr.getType(),
                                                                                    0));
                                } else {
                                    AlgebraicFlag = false;
                                }
                                break;
                            case Instruction::Mul:
                                if (isa<ConstantInt>(Opd1) && ConstVal1 == 1) {
                                    // 1 * x
                                    Instr.replaceAllUsesWith(Opd2);
                                } else if (isa<ConstantInt>(Opd2) && ConstVal2 == 1) {
                                    // x * 1
                                    Instr.replaceAllUsesWith(Opd1);
                                } else {
                                    AlgebraicFlag = false;
                                }
                                break;
                            case Instruction::SDiv:
                                if (isa<Constant>(Opd2) && ConstVal2 == 1) {
                                    // x / 1
                                    Instr.replaceAllUsesWith(Opd1);
                                } else if (Opd1 == Opd2) {
                                    // x / x
                                    Instr.replaceAllUsesWith(ConstantInt::getSigned(Instr.getType(), 1));
                                } else {
                                    AlgebraicFlag = false;
                                }
                                break;
                            default:
                                AlgebraicFlag = false;
                                break;

                        }
                        if (AlgebraicFlag == true) {
                            if (DEBUG) {
                                outs() << "[AL] ";
                                Instr.print(errs());
                                outs() << "\n";
                            }
                            ++AlgebraicOptNum;
                            DeadInstrList.push_back(&Instr);
                        }
                    }
                }
            }
            deleteDeadInstrs(DeadInstrList);
        }

        void constantFold(Function &F) {
            std::vector<Instruction *> DeadInstrList;
            for (auto &BB : F) {
                for (auto &Instr : BB) {
                    if (Instr.getNumOperands() == 2) {
                        int64_t ConstVal1, ConstVal2;
                        Value *Opd1 = Instr.getOperand(0);
                        Value *Opd2 = Instr.getOperand(1);
                        // get int64 signed for ReplaceInstWithValue
                        if (isa<ConstantInt>(Opd1)) {
                            ConstVal1 = dyn_cast<ConstantInt>(Opd1)->getSExtValue();
                        }
                        if (isa<ConstantInt>(Opd2)) {
                            ConstVal2 = dyn_cast<ConstantInt>(Opd2)->getSExtValue();
                        }
                        if (isa<ConstantInt>(Opd1) && isa<ConstantInt>(Opd2)) {
                            //constant folding
                            bool ConstantFoldFlag = true;
                            switch (Instr.getOpcode()) {
                                case Instruction::Add:
                                    Instr.replaceAllUsesWith(
                                            ConstantInt::getSigned(Instr.getType(), ConstVal1 + ConstVal2));
                                    break;
                                case Instruction::Sub:
                                    Instr.replaceAllUsesWith(
                                            ConstantInt::getSigned(Instr.getType(), ConstVal1 + ConstVal2));
                                    break;
                                case Instruction::Mul:
                                    Instr.replaceAllUsesWith(
                                            ConstantInt::getSigned(Instr.getType(), ConstVal1 * ConstVal2));
                                    break;
                                case Instruction::SDiv:
                                    if (ConstVal2 != 0) {
                                        Instr.replaceAllUsesWith(
                                                ConstantInt::getSigned(Instr.getType(), ConstVal1 / ConstVal2));
                                    } else {
                                        ConstantFoldFlag = false;
                                    }
                                    break;
                                default:
                                    ConstantFoldFlag = false;
                            }
                            if (ConstantFoldFlag) {
                                DeadInstrList.push_back(&Instr);
                                if (DEBUG) {
                                    outs() << "[CF] ";
                                    Instr.print(errs());
                                    outs() << "\n";
                                }
                                ++ConstantFoldOptNum;
                            }
                        }
                    }
                }
            }
            deleteDeadInstrs(DeadInstrList);
        }

        void strength(Function &F) {
            std::vector<Instruction *> DeadInstrList;
            for (auto &BB : F) {
                for (auto &Instr : BB) {
                    if (Instr.getNumOperands() == 2) {
                        int64_t ConstVal1, ConstVal2;
                        Value *Opd1 = Instr.getOperand(0);
                        Value *Opd2 = Instr.getOperand(1);
                        // get int64 signed
                        if (isa<ConstantInt>(Opd1)) {
                            ConstVal1 = dyn_cast<ConstantInt>(Opd1)->getSExtValue();
                        }
                        if (isa<ConstantInt>(Opd2)) {
                            ConstVal2 = dyn_cast<ConstantInt>(Opd2)->getSExtValue();
                        }
                        bool StrengthFlag = true;
                        switch (Instr.getOpcode()) {
                            // ignore Add and Sub
                            case Instruction::Mul:
                                if (isa<ConstantInt>(Opd1) && getShift(ConstVal1) != -1) {
                                    // 2^n * x => x << n
                                    Value *v = ConstantInt::getSigned(Instr.getType(), getShift(ConstVal1));
                                    Instr.replaceAllUsesWith(
                                            BinaryOperator::Create(Instruction::Shl, Opd2, v, "shl",
                                                                   &Instr));
                                } else if (isa<ConstantInt>(Opd2) && getShift(ConstVal2) != -1) {
                                    // x * 2^n => x << n
                                    Value *v = ConstantInt::getSigned(Instr.getType(), getShift(ConstVal2));
                                    Instr.replaceAllUsesWith(
                                            BinaryOperator::Create(Instruction::Shl, Opd1, v, "shl",
                                                                   &Instr));
                                } else {
                                    StrengthFlag = false;
                                }
                                break;
                            case Instruction::SDiv:
                                if (isa<ConstantInt>(Opd2) && getShift(ConstVal2) != -1) {
                                    // x / 2^n => x >> n
                                    Value *v = ConstantInt::getSigned(Instr.getType(), getShift(ConstVal2));
                                    Instr.replaceAllUsesWith(
                                            BinaryOperator::Create(Instruction::LShr, Opd1, v, "lshr", &Instr));

                                } else {
                                    StrengthFlag = false;
                                }
                                break;
                            default:
                                StrengthFlag = false;
                                break;
                        }
                        if (StrengthFlag) {
                            if (DEBUG) {
                                outs() << "[ST] ";
                                Instr.print(errs());
                                outs() << "\n";
                            }
                            ++StrengthOptNum;
                            DeadInstrList.push_back(&Instr);
                        }
                    }
                }
            }
            deleteDeadInstrs(DeadInstrList);
        }
    };
}

char LocalOpts::ID = 0;
static RegisterPass<LocalOpts> X("local-opts", "15745: LocalOpts",
                                 false /* Only looks at CFG */,
                                 false /* Analysis Pass */);