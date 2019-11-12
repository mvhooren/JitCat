project(JitCatValidatorTool)

include_directories(
	${JitCatValidatorIncludePath}
	${JitCatValidatorToolIncludePath}
)

set(JitCatValidatorToolSourceFiles JitCatValidatorTool.cpp)
	
source_group("" FILES ${JitCatValidatorToolSourceFiles})

add_executable(JitCatValidatorTool ${JitCatValidatorToolSourceFiles})
	
add_dependencies(JitCatValidatorTool JitCatValidator)

target_link_libraries(JitCatValidatorTool JitCatValidator)
					  
if(UNIX)
	set_target_properties(JitCatValidatorTool PROPERTIES INSTALL_RPATH "./")
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER JitCat)