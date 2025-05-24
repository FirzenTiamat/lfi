#!/bin/sh
set -x
set -e

PREFIX=$PWD/aarch64-lfi-clang
ARCH=aarch64-lfi

DEFINE_FLAGS="-DLFI_DEFAULT_FLAGS='\"$LFIFLAGS\"'"

mkdir build-llvm-attsfi-llc
cd build-llvm-attsfi-llc
cmake -G Ninja ../llvm-project-attsfi-llc/llvm \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DLLVM_ENABLE_PROJECTS="lld;clang" \
    -DLLVM_TARGETS_TO_BUILD="X86;AArch64;WebAssembly" \
    -DLLVM_DEFAULT_TARGET_TRIPLE="$ARCH-linux-musl" \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DLLVM_BUILD_DOCS=OFF \
    -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON \
    -DCLANG_DEFAULT_RTLIB="compiler-rt" \
    -DCLANG_DEFAULT_CXX_STDLIB="libc++" \
    -DCLANG_DEFAULT_UNWINDLIB="libunwind" \
    -DCLANG_DEFAULT_LINKER="lld" \
    -DCLANG_DEFAULT_OBJCOPY="llvm-objcopy" \
    -DCMAKE_C_FLAGS="$DEFINE_FLAGS" \
    -DCMAKE_CXX_FLAGS="$DEFINE_FLAGS" \
    -DDEFAULT_SYSROOT=../sysroot
ninja llc

#cd ..
#cp build-llvm-attsfi-llc/bin/llc $PREFIX/bin/llc.attsfi