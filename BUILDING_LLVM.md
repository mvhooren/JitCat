# Building LLVM for use with JitCat
This document describes how to configure a LLVM http://llvm.org/ build for use with JitCat.

## Important
Due to an issue issue with LLVM, before building, we need to apply a small workaround to the llvm code when building LLVM for Windows/MSVC:  
Find MCAsmInfoCOFF.cpp and change HasCOFFComdatConstants to false.  
See this issue on the LLVM bugzilla: https://bugs.llvm.org/show_bug.cgi?id=40074  

## Introduction
JitCat uses some of the latest features of LLVM, some of which are not yet included in an official LLVM release.  
Therefore, you will have to build LLVM from source. Check out the trunk using SVN:  
http://llvm.org/svn/llvm-project/llvm/trunk  
JitCat will track the trunk as much as possible, but it can happen that updates to LLVM will break a JitCat build.  
The last time this document was updated, JitCat was built against revision 357703.  
Once a stable LLVM version is released that includes everything JitCat needs, JitCat will target that version.  

## Read the documentation
For building LLVM first of all refer to LLVM build documentation:  
Getting started https://llvm.org/docs/GettingStarted.html  
Building LLVM with Cmake https://llvm.org/docs/CMake.html  
Getting Started with the LLVM System using Microsoft Visual Studio https://llvm.org/docs/GettingStartedVS.html  

## Non-Windows/MSVC platforms
The default CMake configuration of LLVM will work fine, but it is possible to reduce build time and diskspace usage of the LLVM build by disabling certain features (see below).

## Windows / Visual Studio
Use CMake to generate your Visual Studio project files. (I highly recommend using the CMake gui.)  
Make sure you select the proper project generator for your use case. There are separate generators for 64 bit and 32 bit projects.  

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