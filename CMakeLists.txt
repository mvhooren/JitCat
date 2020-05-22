#Requires CMake 3.12 because of add_compile_definitions 
cmake_minimum_required(VERSION 3.12)

project(JitCat)
#These are all the options available for configuration using CMake
option(LLVM_ENABLED "Enable the LLVM backend for JitCat." OFF)
option(DEBUG_GRAMMAR "Will print lots of debug output for the parser generator." OFF)
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

#Set up some variables that are used later on
if(CMAKE_SIZEOF_VOID_P MATCHES "8" AND NOT(FORCE32))
	set(JITCAT_BITNESS "64")
else()
	set(JITCAT_BITNESS "32")
endif()

set(JitCatIncludePath ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(JitCatValidatorIncludePath ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(JitCatValidatorToolIncludePath ${CMAKE_CURRENT_SOURCE_DIR}/tools/jitcatvalidatortool)
set(BasicExampleIncludePath ${CMAKE_CURRENT_SOURCE_DIR}/examples/basicexample)
set(JitCatUnitTestsIncludePath ${CMAKE_CURRENT_SOURCE_DIR}/test/jitcatunittests)
set(DependenciesIncludePath ${CMAKE_CURRENT_SOURCE_DIR}/dependencies)

if (DEBUG_GRAMMAR)
	add_compile_definitions(DEBUG_GRAMMAR)
endif()

if(LLVM_ENABLED)
	add_compile_definitions(ENABLE_LLVM)
	include_directories(${LLVM_INCLUDES})
	include_directories(${LLVM_BUILD_DIR}/include)
	if (MSVC)
		if (${CMAKE_GENERATOR} STREQUAL "Ninja") 
		message("Ninja generator")
		link_directories(${LLVM_BUILD_DIR}/lib)
		else()
		message("Generator: ${CMAKE_GENERATOR}")
		link_directories(${LLVM_BUILD_DIR}/$(ConfigurationName)/lib)
		endif()
	else()
		link_directories(${LLVM_BUILD_DIR}/lib)
	endif()	
	
	link_libraries(
		LLVMX86Disassembler
		LLVMX86CodeGen
		LLVMX86AsmParser
		LLVMX86Desc
		LLVMX86Info
		LLVMX86Utils
		LLVMSelectionDAG
		LLVMOrcJIT
		LLVMOrcError
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
		LLVMCFGuard
		LLVMTransformUtils
		LLVMTarget
		LLVMAnalysis
		LLVMProfileData
		LLVMMCParser
		LLVMMC
		LLVMDebugInfoCodeView
		LLVMDebugInfoDWARF
		LLVMDebugInfoMSF
		LLVMObject
		LLVMBitReader
		LLVMBitstreamReader
		LLVMCore
		LLVMBinaryFormat
		LLVMSupport
		LLVMDemangle
		LLVMRemarks
		LLVMTextAPI
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
	add_compile_options("/bigobj")
	if (NOT ${CMAKE_GENERATOR} STREQUAL "Ninja") 
		add_compile_options("/MP")
		#optionally enable the experimental msvc feature that gets rid of lots of warnings from llvm
		if(MSVC_EXPERIMENTAL_DISABLE_EXTERNAL_WARNINGS)
			add_compile_options(/experimental:external /external:anglebrackets /external:W0)
		endif()
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
	
	install(DIRECTORY include/jitcat 
			DESTINATION include
			COMPONENT jitcat-headers
			FILES_MATCHING
			PATTERN "*.h"
			PATTERN ".svn" EXCLUDE
	)	
endif()

if(BUILD_VALIDATOR_LIB)
	list(APPEND AllTargets JitCatValidator)
	
	add_subdirectory(src/jitcatvalidator)
	
	install(DIRECTORY include/jitcatvalidator 
			DESTINATION include
			COMPONENT jitcatvalidator-headers
			FILES_MATCHING
			PATTERN "*.h"
			PATTERN ".svn" EXCLUDE
	)	
endif()

if(BUILD_EXAMPLES)
	list(APPEND AllTargets BasicExample)
	add_subdirectory(examples/basicexample)
endif()

if(BUILD_UNIT_TESTS)
	list(APPEND AllTargets JitCatUnitTests)
	add_subdirectory(test/jitcatunittests)
endif()

if(BUILD_VALIDATOR_TOOL)
	list(APPEND AllTargets JitCatValidatorTool)
	add_subdirectory(tools/jitcatvalidatortool)
endif()

#Set C++ 17 standard on all targets
set_target_properties(${AllTargets} PROPERTIES CXX_STANDARD 17)
if (NOT MSVC)
	set_target_properties(${AllTargets} PROPERTIES LINK_FLAGS_RELEASE "-Wl,-s,-x")	
endif()

install(TARGETS ${AllTargets}
		RUNTIME DESTINATION bin/JitCat${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		LIBRARY DESTINATION lib/JitCat${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		ARCHIVE DESTINATION lib/JitCat${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})
