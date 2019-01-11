cmake_minimum_required(VERSION 3.12)

option(LLVM_ENABLED "Enable the LLVM backend for JitCat." OFF)
option(BUILD_JITCAT_LIB "Build the JitCat library." ON)
option(BUILD_VALIDATOR_LIB "Build the JitCat validator library." ON)
option(BUILD_VALIDATOR_TOOL "Build the JitCat validator tool." ON)
option(BUILD_UNIT_TESTS "Build JitCat unit tests." ON)
option(BUILD_EXAMPLES "Build JitCat example programs." ON)
set(LLVM_INCLUDES "" CACHE PATH "LLVM include path.")
set(LLVM_BUILD_DIR "" CACHE PATH "Path where LLVM was built. This should contain an include and lib folder. On Windows, it should contain Release/Debug/RelWithDebInfo folders.")
if(NOT MSVC)
	option(FORCE32 "Force a 32bit compile on 64bit" OFF)
	if (NOT CMAKE_BUILD_TYPE)
	 set(CMAKE_BUILD_TYPE "Release")
	endif ()
else()
	option(MSVC_EXPERIMENTAL_DISABLE_EXTERNAL_WARNINGS "Enable the experimental msvc feature to disable warnings from headers included using angle brackets <>" OFF)
endif()

if(CMAKE_SIZEOF_VOID_P MATCHES "8" AND NOT(FORCE32))
	set(JITCAT_BITNESS "64")
else()
	set(JITCAT_BITNESS "32")
endif()

set(JitCatIncludePath ${CMAKE_CURRENT_SOURCE_DIR}/include/jitcat)
set(JitCatValidatorIncludePath ${CMAKE_CURRENT_SOURCE_DIR}/include/jitcatvalidator)
set(DependenciesIncludePath ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

add_compile_definitions(NO_DEBUG_GRAMMAR)

if(LLVM_ENABLED)
	add_compile_definitions(ENABLE_LLVM)
	include_directories(${LLVM_INCLUDES})
	include_directories(${LLVM_BUILD_DIR}/include)
	if (MSVC)
		link_directories(${LLVM_BUILD_DIR}/$(Configuration)/lib)
	else()
		link_directories(${LLVM_BUILD_DIR}/lib)
	endif()	
	
	link_libraries(
		LLVMX86Disassembler
		LLVMX86CodeGen
		LLVMX86AsmParser
		LLVMX86Desc
		LLVMX86Info
		LLVMX86AsmPrinter
		LLVMX86Utils
		LLVMSelectionDAG
		LLVMOrcJIT
		LLVMMCDisassembler
		LLVMGlobalISel
		LLVMExecutionEngine
		LLVMRuntimeDyld
		LLVMAsmPrinter
		LLVMCodeGen
		LLVMScalarOpts
		LLVMInstCombine
		LLVMBitWriter
		LLVMAggressiveInstCombine
		LLVMTransformUtils
		LLVMTarget
		LLVMAnalysis
		LLVMProfileData
		LLVMObject
		LLVMMCParser
		LLVMMC
		LLVMDebugInfoCodeView
		LLVMDebugInfoMSF
		LLVMBitReader
		LLVMCore
		LLVMBinaryFormat
		LLVMSupport
		LLVMDemangle
	)
	if (NOT MSVC)
		link_libraries(
			z
			rt
			dl
			tinfo
			pthread
			m
			xml2		
		)
	endif()
endif()

if(MSVC)
	#use 64bit toolchain
	set(CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE x64)
	#disable iterator base class deprication warning
	add_compile_definitions(_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)
	#Replace each instance of /MD with /MT and /EHsc with /EHa
	foreach(flag_var
			CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
	   if(${flag_var} MATCHES "/MD")
		  string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	   endif(${flag_var} MATCHES "/MD")
	   if(${flag_var} MATCHES "/EHsc")
		  string(REGEX REPLACE "/EHsc" "/EHa" ${flag_var} "${${flag_var}}")
	   endif(${flag_var} MATCHES "/EHsc")	   
	endforeach(flag_var)
	#always enable multi processor compilation
	add_compile_options("/MP")
	#optionally enable the experimental msvc feature that gets rid of lots of warnings from llvm
	if(MSVC_EXPERIMENTAL_DISABLE_EXTERNAL_WARNINGS)
		add_compile_options(/experimental:external /external:anglebrackets /external:W0)
	endif()
	
	#Set the startup project for
	if (BUILD_UNIT_TESTS)
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT JitCatUnitTests)
	elseif(BUILD_EXAMPLES)
		set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT JitCatTest)
	endif()	
else()
	add_compile_options(-Wno-write-strings -Wno-switch -Wno-unused-function -fno-rtti)
	if(FORCE32)
		add_compile_options("-m32")
	endif()
endif()

set(AllTargets "") 

if(BUILD_JITCAT_LIB)
	list(APPEND AllTargets JitCat) 
	add_subdirectory(src/jitcat)
endif()

if(BUILD_VALIDATOR_LIB)
	list(APPEND AllTargets JitCatValidator)
	add_subdirectory(src/jitcatvalidator)
endif()

if(BUILD_EXAMPLES)
	list(APPEND AllTargets JitCatTest)
	add_subdirectory(test/jitcattest)
endif()

if(BUILD_UNIT_TESTS)
	list(APPEND AllTargets JitCatUnitTests)
	add_subdirectory(test/jitcatunittests)
endif()

if(BUILD_VALIDATOR_TOOL)
	list(APPEND AllTargets JitCatValidatorTool)
	add_subdirectory(tools/jitcatvalidatortool)
endif()


install(TARGETS ${AllTargets}
		RUNTIME DESTINATION bin/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		LIBRARY DESTINATION lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		ARCHIVE DESTINATION lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})