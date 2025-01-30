#include "TracerPass.h"
#include <sstream>
using namespace llvm;
// void LegacyTracerPass::getAnalysisUsage(AnalysisUsage &AU) const {
// 	AU.setPreservesAll();
// }

void TracerPass::instrumentCall(Module &module, CallInst &CI){
	Value *callTarget = NULL;
	if (CI.getCalledFunction() && !CI.getCalledFunction()->isIntrinsic()){
		callTarget = CI.getCalledOperand()->stripPointerCasts();
	}
	else if (!CI.getCalledFunction() && CI.getCalledOperand()){
		callTarget = CI.getCalledOperand()->stripPointerCasts();
	}
	if (!callTarget || isa<InlineAsm>(callTarget)){
		return;
	}
	LLVMContext &CTX = module.getContext();
	IRBuilder<> builder(&CI);
	FunctionCallee trace_flow_call = get_trace_flow_call_func(module);
	Value* callCasted = callTarget;
	if(NO_OPAQUE_POINTER){
		callCasted = builder.CreateBitCast(callTarget, PointerType::getUnqual(CTX));
	}
	auto tc = builder.CreateCall(trace_flow_call, {callCasted});
	MDNode *N = MDNode::get(CTX, MDString::get(CTX, this->passtag));
	tc->setMetadata(this->instTag, N);
	return;
}

void TracerPass::instrumentRet(Module &module, ReturnInst &RI){
	LLVMContext &CTX = module.getContext();
	IRBuilder<> builder(&RI);
	FunctionCallee trace_flow_ret = get_trace_flow_ret_func(module);
	Value *retTarget = NULL;
	Function* intrinsic_retaddr = Intrinsic::getDeclaration(&module, Intrinsic::returnaddress, {});
	assert(intrinsic_retaddr);
	retTarget = builder.CreateCall(intrinsic_retaddr, {ConstantInt::get(Type::getInt32Ty(CTX), 0)});
	Value* retCasted = retTarget;
	if(NO_OPAQUE_POINTER){
		retCasted = builder.CreateBitCast(retTarget, PointerType::getUnqual(CTX));
	}
	auto tc = builder.CreateCall(trace_flow_ret, {retCasted});
	MDNode *N = MDNode::get(CTX, MDString::get(CTX, this->passtag));
	tc->setMetadata(this->instTag, N);
	return;
}

void TracerPass::instrumentIndirectBr(Module &module, IndirectBrInst &IBI){
	Value *indirectbrTarget = NULL;
	indirectbrTarget = IBI.getAddress()->stripPointerCasts();
	if(!indirectbrTarget){
		return;
	}
	LLVMContext &CTX = module.getContext();
	IRBuilder<> builder(&IBI);
	FunctionCallee trace_flow_indirectbr = get_trace_flow_indirectbr_func(module);
	Value* indirectbrCasted = indirectbrTarget;
	if(NO_OPAQUE_POINTER){
		indirectbrCasted = builder.CreateBitCast(indirectbrTarget, PointerType::getUnqual(CTX));
	}
	auto tc = builder.CreateCall(trace_flow_indirectbr, {indirectbrCasted});
	MDNode *N = MDNode::get(CTX, MDString::get(CTX, this->passtag));
	tc->setMetadata(this->instTag, N);
	return;
}

void TracerPass::instrumentEdgeFunc(Module &module, Function &F){
	LLVMContext &CTX = module.getContext();
	IRBuilder<> builder(&*F.getEntryBlock().getFirstInsertionPt());
	auto tc = builder.CreateCall(get_first_flush_func(module), {});
	MDNode *N = MDNode::get(CTX, MDString::get(CTX, this->passtag));
	tc->setMetadata(this->instTag, N);
	return;
}

bool TracerPass::doTrace(Function &FF)
{
	Function *currFunc = &FF;
	Module *m = currFunc->getParent();
	StringRef currFuncNameStr = currFunc->getName();

	// candidate prev debug location, in case prevInst is null or no debug location
	// DebugLoc candidateDL = DebugLoc();

	for (Function::iterator b = currFunc->begin(); b != currFunc->end(); b++){
		BasicBlock &BB = *b;
		for (BasicBlock::iterator instIt = BB.begin(); instIt != BB.end(); instIt++)
		{
			Instruction &inst = *instIt;
			if (auto *CI = dyn_cast<CallInst>(&inst)){
				instrumentCall(*m, *CI);
			}
			else if (auto *RI = dyn_cast<ReturnInst>(&inst)){
				instrumentRet(*m, *RI);
			}
			else if (auto *IBI = dyn_cast<IndirectBrInst>(&inst)){
				instrumentIndirectBr(*m, *IBI);
			}
		}
	}
	// instrument entry
	if (edge_funcs.count(currFuncNameStr.str())){
		instrumentEdgeFunc(*m, *currFunc);
	}
	return true;
}


PreservedAnalyses TracerPass::run(Module &module, ModuleAnalysisManager &)
{
	readEdgeFuncList();
	// readExitFuncList();
	readSkipFuncList();
	for (auto &F: module){
		if (isSkipInstrumentation(&F)){
			continue;
		}
		doTrace(F);
	}
	return PreservedAnalyses::all();
}

llvm::PassPluginLibraryInfo getTracerPassPluginInfo()
{
	return {
		LLVM_PLUGIN_API_VERSION,
		"TracerPass",
		LLVM_VERSION_STRING,
		[](PassBuilder &PB)
		{
			PB.registerPipelineParsingCallback(
				[](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>)
				{
					if (Name == "tracer-pass")
					{
						MPM.addPass(TracerPass());
						return true;
					}
					return false;
				});
			PB.registerPipelineStartEPCallback(
				[](ModulePassManager &MPM, OptimizationLevel Level)
				{
					MPM.addPass(TracerPass());
				});
		}};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo(){
	return getTracerPassPluginInfo();
}

// bool LegacyTracerPass::runOnModule(Module &module){
// 	tracerPassImpl.readEdgeFuncList();
// 	tracerPassImpl.readExitFuncList();
// 	tracerPassImpl.readSkipFuncList();
// 	for (auto &F: module){
// 		if (tracerPassImpl.isSkipInstrumentation(&F)){
// 			continue;
// 		}
// 		tracerPassImpl.doTrace(F);
// 	}
// 	return false;
// }
// char LegacyTracerPass::ID = 0;


// static RegisterPass<LegacyTracerPass>
//     X("tracer-pass", "LegacyTracerInstrumentionPass", false, false);

