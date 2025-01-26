cmake_minimum_required(VERSION 3.16)
project(TracerPass)
find_package(LLVM REQUIRED CONFIG)

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
SET(CMAKE_CXX_FLAGS "-Wall -fno-rtti -g -O3")

add_subdirectory(InstPass)
# add_subdirectory(AfterLinkOptPass)

target_link_libraries(LLVMTracerPass PUBLIC)