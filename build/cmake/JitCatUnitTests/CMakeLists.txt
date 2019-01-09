project(JitCatUnitTests)

include_directories(
    ../../../include/jitcat
	../../../dependencies
	../../../test/jitcatunittests
)

file(GLOB JITCATUNITTESTS_SOURCE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ../../../test/jitcatunittests/*.h
    ../../../test/jitcatunittests/*.cpp
)

add_executable(JitCatUnitTests ${JITCATUNITTESTS_SOURCE})
add_dependencies(JitCatUnitTests JitCat)

link_directories(${CMAKE_SOURCE_DIR}/../../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})
link_directories(${LLVM_LIBS})
target_link_libraries(JitCatUnitTests 
	JitCat
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
	z
	rt
	dl
	tinfo
	pthread
	m
	xml2
)
set_target_properties(JitCatUntTests
    PROPERTIES
    COMPILE_FLAGS "-std=c++17 -Wno-write-strings -Wno-switch -Wno-unused-function"
    LINK_FLAGS_RELEASE "-Wl,-s,-x"
)

install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/../../bin/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})
