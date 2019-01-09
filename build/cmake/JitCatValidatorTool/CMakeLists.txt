project(JitCatValidatorTool)

include_directories(
	../../../include/jitcatvalidator
	../../../tools/jitcatvalidatortool
)

file(GLOB JITCATVALIDATORTOOL_SOURCE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ../../../tools/jitcatvalidatortool/*.h
    ../../../tools/jitcatvalidatortool/*.cpp
)

add_executable(JitCatValidatorTool ${JITCATVALIDATORTOOL_SOURCE})
add_dependencies(JitCatValidatorTool JitCatValidator)

link_directories(${CMAKE_SOURCE_DIR}/../../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})
link_directories(${LLVM_LIBS})
target_link_libraries(JitCatValidatorTool 
	JitCatValidator
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

set_target_properties(JitCatValidatorTool
    PROPERTIES
    COMPILE_FLAGS "-std=c++17 -Wno-write-strings -Wno-switch -Wno-unused-function -fno-rtti"
    LINK_FLAGS_RELEASE "-Wl,-s,-x"
	INSTALL_RPATH "./"
)

install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/../../bin/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})
