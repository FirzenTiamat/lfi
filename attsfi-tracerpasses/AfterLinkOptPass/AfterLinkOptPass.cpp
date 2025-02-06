#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
// #include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/InlineAsm.h"

#ifndef CALLTRACER
#define CALLTRACER "tracecall"
#endif
#ifndef RETTRACER
#define RETTRACER "traceret"
#endif

#ifndef DEBUG_TYPE
#define DEBUG_TYPE "tracer-pass"
#endif

using namespace llvm;

namespace {
  class AfterLinkOptPass : public PassInfoMixin<AfterLinkOptPass> {
  public:
	StringRef getPassName(){
		return "After Instrumentation and Link Opt Pass";
	}
	static bool isRequired() {return true;}

	PreservedAnalyses run(Module &module, ModuleAnalysisManager &){
    for (Function &F : module){
      runOnFunction(F);
    }
    return PreservedAnalyses::all();
  }

    bool runOnFunction(Function &F) {
      size_t count_omited_tracecall = 0;
      size_t count_omited_traceret = 0;
      for (Function::iterator BBit = F.begin(); BBit != F.end(); BBit++){
        BasicBlock &bb = *BBit;
        for (BasicBlock::iterator instIt = bb.begin(); instIt != bb.end(); instIt++){
            Instruction &inst = *instIt;
            if (isa<IntrinsicInst>(inst) || !isa<CallInst>(inst)){
                continue;
            }
            // FunctionCallee instedCallee = dyn_cast<CallInst>(&inst)->getCalledFunction();
            // StringRef funcName = instedCallee.getCallee()->getName();
            Function* calledFunction = dyn_cast<CallInst>(&inst)->getCalledFunction();
            if (!calledFunction){
                continue;
            }
            StringRef funcName = calledFunction->getName();
            //errs() << "Function: " << F.getName() << " inst: " << inst.getOpcodeName() << " " << funcName << "\n";
            if (funcName.compare(RETTRACER) && funcName.compare(CALLTRACER)){
                continue;
            }
            //omit traceret() but next instruction is not ret
            if (!funcName.compare(RETTRACER) && !isa<ReturnInst>(inst.getNextNode())){
                LLVM_DEBUG(dbgs() << "\tOmitted: " << F.getName() << ":" << bb.getName() << ":traceret("<< inst.getOperand(0)->getNameOrAsOperand() <<")\n");
                instIt = inst.eraseFromParent();
                count_omited_traceret++;
                continue; //F return
            }
            //omit tracecall(target, ...) but next instruction is not the traget call
            Instruction *nextInst = inst.getNextNode();  //should be the right traced call
            if (!funcName.compare(CALLTRACER)){
                Value *tracedTarget = inst.getOperand(0)->stripPointerCasts();
                if (isa<CallInst>(nextInst) && dyn_cast<CallInst>(nextInst)->getCalledOperand()->stripPointerCasts() == tracedTarget){
                    continue;
                }
                LLVM_DEBUG(dbgs() << "\tOmitted: " << F.getName() << ":" << bb.getName() << ":tracecall(" << inst.getOperand(0)->getNameOrAsOperand() << ")\n");
                instIt = inst.eraseFromParent();
                count_omited_tracecall++;
                continue;
            }
        }
      }
      if(count_omited_tracecall + count_omited_traceret > 0){
        errs() << "Function: " << F.getName()  << " omitted trace call/ret pairs=<" << count_omited_tracecall <<  ", " << count_omited_traceret << ">\n";
      }
      return false;
    }
  };
}


//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
llvm::PassPluginLibraryInfo getAfterLinkOptPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "AfterLinkOptPass", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "after-link-opt-pass") {
                    MPM.addPass(AfterLinkOptPass());
                    return true;
                  }
                  return false;
                });
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                  MPM.addPass(AfterLinkOptPass());
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getAfterLinkOptPassPluginInfo();
}