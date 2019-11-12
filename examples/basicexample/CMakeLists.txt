project(BasicExample)

include_directories(
    ${JitCatIncludePath}
	${BasicExampleIncludePath}
)


set(BasicExampleSourceFiles
	ReflectionTestObject.cpp
	ReflectionTestObject2.cpp
	ReflectionTestRoot.cpp
	BasicExample.cpp
)

set(BasicExampleHeaderFiles
	ReflectionTestRoot.h
	ReflectionTestObject.h
	ReflectionTestObject2.h
)
	
set_source_files_properties(${BasicExampleHeaderFiles} PROPERTIES HEADER_FILE_ONLY ON)

source_group("" FILES 	
	${BasicExampleSourceFiles}
	${BasicExampleHeaderFiles}
)

add_executable(BasicExample 
	${BasicExampleSourceFiles}
	${BasicExampleHeaderFiles}
)

add_dependencies(BasicExample JitCat)

target_link_libraries(BasicExample JitCat)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER JitCat)