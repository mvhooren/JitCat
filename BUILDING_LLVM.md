# Building LLVM for use with JitCat
This document describes how to configure a LLVM http://llvm.org/ build for use with JitCat.

## Introduction
JitCat uses the [LLVM 10.0.0](https://github.com/llvm/llvm-project/releases/tag/llvmorg-10.0.0) release that you will have to build from source. The binary release cannot be used because it is compiled with the C++14 standard and JitCat is targeting C++17.
Download the source from github: https://github.com/llvm/llvm-project/archive/llvmorg-10.0.0.zip

## Read the documentation
For building LLVM first of all refer to LLVM build documentation:  
[Getting started](https://llvm.org/docs/GettingStarted.html)  
[Building LLVM with Cmake](https://llvm.org/docs/CMake.html)  
[Getting Started with the LLVM System using Microsoft Visual Studio](https://llvm.org/docs/GettingStartedVS.html)  

## All Platforms
Set CMAKE_CXX_STANDARD to 17. LLVM currently targets C++14 but will build under C++17 as well. Building LLVM with C++14 will possibly lead to incompatibilities. For example, see [this issue](https://github.com/mvhooren/JitCat/issues/13).

## Non-Windows/MSVC platforms
Except for the CMAKE_CCXX_STANDARD, the default CMake configuration of LLVM will work fine. It is possible to greatly reduce build time and diskspace usage of the LLVM build by disabling certain features (see below).

## Windows / Visual Studio
Use CMake to generate your Visual Studio project files. (I highly recommend using the CMake gui.)  
Make sure you select the proper project generator for your use case. There are separate generators for 64 bit and 32 bit projects.  

Add /D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to CMAKE_CXX_FLAGS. Without this MSVC will generate an enormous amount of deprecation warnings and a few errors as well.

By default, JitCat is built with /MT (Multithread - statically linked runtime library), however LLVM is built with /MD (Multithreaded DLL runtime library) by default.  
Either change JitCat to /MD if you are ok with the external dependency on a C++ runtime DLL, or change LLVM to build with /MT.  
In order to do that, change MD to MT and MDd to MTd in the following settings:  
LLVM_USE_CRT_DEBUG  
LLVM_USE_CRT_MINSIZEREL  
LLVM_USE_CRT_RELEASE  
LLVM_USE_CRT_RELWITHDEBINFO  
CMAKE_CXX_FLAGS  
CMAKE_CXX_FLAGS_DEBUG  
CMAKE_CXX_FLAGS_MINSIZEREL  
CMAKE_CXX_FLAGS_RELWITHDEBINFO  
CMAKE_C_FLAGS  
CMAKE_C_FLAGS_DEBUG  
CMAKE_C_FLAGS_MINSIZEREL  
CMAKE_C_FLAGS_RELWITHDEBINFO  


## Reducing LLVM build time (all platforms)

LLVM is a huge project and JitCat only needs part of it.  
In order to minimize build time, a lot of things can be disabled.  

Disable tools, tests, benchmarks, etc by turning off these options:  
LLVM_INCLUDE_BENCHMARKS  
LLVM_INCLUDE_DOCS  
LLVM_INCLUDE_EXAMPLES  
LLVM_INCLUDE_GO_TESTS  
LLVM_INCLUDE_TEST  
LLVM_INCLUDE_TOOLS  
LLVM_INCLUDE_UTILS  
  
LLVM can generate code for many target platforms, but we only need one.  
For JitCat, select the same target for which you are building JitCat.  
Most likely, this should be X86.  
Change LLVM_TARGETS_TO_BUILD from 'all' to your target platform.  
