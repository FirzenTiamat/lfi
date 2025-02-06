project(llvm-afterlinkopt)

add_library(afterlink SHARED 
    AfterLinkOptPass.cpp
)

set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
# SET(CMAKE_CXX_FLAGS "-Wall -fno-rtti -g -O3")

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always -Wno-everything -fPIC")

# LLVM is (typically) built with no C++ RTTI. We need to match that.
set_target_properties(afterlink PROPERTIES
    COMPILE_FLAGS "-fno-rtti"
)

