//
// Created by sakura on 2020/7/9.
//

#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;


namespace {

    class FunctionInfo final : public ModulePass {
    public:
        static char ID;

        FunctionInfo() : ModulePass(ID) {}

        virtual ~FunctionInfo() override {}

        // We don't modify the program, so we preserve all analysis.
        virtual void getAnalysisUsage(AnalysisUsage &AU) const override {
            AU.setPreservesAll();
        }

        virtual bool runOnModule(Module &M) override {
            outs() << "CSCD70 Functions Information Pass" << "\n";

            // @TODO Please complete this function.
            outs() << M.getName() << "\n";
            outs() << "Name    " << "# Args    " << "# Calls    " << "# Blocks    " << "# Insts    " << "\n";
            for(auto &F : M){
                outs() << F.getName() << "    ";
                outs() << F.arg_size() << "    ";
                outs() << F.getNumUses() << "    ";
                outs() << F.getBasicBlockList().size() << "    ";
                int InstrSize = 0;
                for(auto &BB : F){
                    InstrSize += BB.getInstList().size();
                }
                outs() << InstrSize << "    " << "\n";
            }
            return false;
        }
    };

    char FunctionInfo::ID = 0;
    RegisterPass<FunctionInfo> X(
            "function-info",
            "CSCD70: Functions Information");

}  // namespace anonymous