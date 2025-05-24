
# Low level instrumentation to trace accurate return address.
# this backend pass have to be integrited into the llvm project.

# add target in {llvm-project-dir}/llvm/lib/Target/AArch64/CMakeLists.txt:
# add_llvm_target( ... AArch64AttsfiTraceret.cpp ...)

# add file AArch64AttsfiTraceret.cpp into dir {llvm-project-dir}/llvm/lib/Target/AArch64/

# add below into {llvm-project-dir}/llvm/lib/Target/AArch64/AArch64.h:
# FunctionPass *createAArch64AttsfiTraceretPass();
# void initializeAArch64AttsfiTraceretPass(PassRegistry&);

# add below into {llvm-project-dir}/llvm/lib/Target/AArch64/AArch64TargetMachine.cpp:
# addPass(createAArch64AttsfiTraceretPass());
# at the end of function: void AArch64PassConfig::addPreEmitPass() {...}

# a great reference: https://www.kharghoshal.xyz/blog/writing-machinefunctionpass

# refer to build-attsfi.sh to build the llc with the traceret pass,
# in the same directory in lfi lfi-llvm-toolchain's builds scripts. 

# last, remove the traceret instrumentation at IR level in InstPass/TracerPass.cpp