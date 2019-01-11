project(JitCatValidatorTool)

include_directories(
	${JitCatValidatorIncludePath}
	../../tools/jitcatvalidatortool
)


set(JitCatValidatorToolSourceFiles
	JitCatValidatorTool.cpp
)
	
source_group("" FILES 	
	${JitCatValidatorToolSourceFiles}
)

add_executable(JitCatValidatorTool 
	${JitCatValidatorToolSourceFiles}
)
	
add_dependencies(JitCatValidatorTool JitCatValidator)

link_directories(${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})

target_link_libraries(JitCatValidatorTool 
					  JitCatValidator)
					  
#Set C++ 17 standard					  
set_property(TARGET JitCatValidatorTool PROPERTY CXX_STANDARD 17)
					  
if(UNIX)
set_target_properties(JitCatValidatorTool
    PROPERTIES
    LINK_FLAGS_RELEASE "-Wl,-s,-x"
	INSTALL_RPATH "./"
)
endif()
