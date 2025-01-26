project(llvm-InstPass)

set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")




add_library( LLVMTracerPass SHARED 
  TracerPass.cpp
  TracerPass.h
  )
