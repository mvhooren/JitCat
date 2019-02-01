# Building JitCat
JitCat uses the (CMake)[http://www.cmake.org] build system on all supported platforms.
Using CMake, you can generate a makefile or project files for IDEs like Visual Studio.
I recommend using the CMake GUI.

## Prerequisites
- CMake 3.12 or later
- A C++17 compatible C++ compiler, for example GCC 8.0, Clang 6.0 or Visual Studio 2017
- (Optional) A build of LLVM

## Enabling/Disabling building JitCat components
There are several CMake options for enabling/disabling building parts of JitCat. These are all enabled by default.
BUILD_EXAMPLES 
BUILD_JITCAT_LIB 
BUILD_UNIT_TESTS 
BUILD_VALIDATOR_LIB
BUILD_VALIDATOR_TOOL

If you just need the library and not any of the tools/tests/examples, only enable BUILD_JITCAT_LIB and disable the rest.

## Enable LLVM support
JitCat has an optional dependency on [LLVM](http://www.llvm.org).
This allows JitCat to build expressions to native code instead of executing expressions through the interpreter and improves performance of non-trivial expressions by a lot.
See [this document](BUILDING_LLVM.md) for information on how to build LLVM for use with JitCat.
To enable the use of LLVM with JitCat, set the following variables when generating build files with CMake:
LLVM_ENABLED <- on
LLVM_BUILD_DIR <- the directory where LLVM was built, this is usually the output directory set with CMake when building LLVM.
LLVM_INCLUDES <- the directory where LLVM include files are located, this is usually the "include" folder inside a LLVM checkout.

LLVM_ENABLED must also be defined in your program before including any JitCat headers.

You will also need to link to the LLVM libraries in addition to the JitCat library.
See the [CMakeLists.txt](CMakeLists.txt) file for which LLVM libraries to link against. The order in which those libraries are linked is critical.

## Other CMake options
When generating Visual Studio project files there is an additional option that enables an experimental feature in Visual Studio that disables warnings from headers external to JitCat:
MSVC_EXPERIMENTAL_DISABLE_EXTERNAL_WARNINGS
It is recommeded to enable this when building with LLVM support because the LLVM headers currently generate a lot of warnings in MSVC.
