project(JitCatTest)

include_directories(
    ${JitCatIncludePath}
	${JitCatTestIncludePath}
)


set(JitCatTestSourceFiles
	ReflectionTestObject.cpp
	ReflectionTestObject2.cpp
	ReflectionTestRoot.cpp
	JitCatTest.cpp
)

set(JitCatTestHeaderFiles
	ReflectionTestRoot.h
	ReflectionTestObject.h
	ReflectionTestObject2.h
)
	
set_source_files_properties(${JitCatTestHeaderFiles} PROPERTIES HEADER_FILE_ONLY ON)

source_group("" FILES 	
	${JitCatTestSourceFiles}
	${JitCatTestHeaderFiles}
)

add_executable(JitCatTest 
	${JitCatTestSourceFiles}
	${JitCatTestHeaderFiles}
)

add_dependencies(JitCatTest JitCat)

target_link_libraries(JitCatTest JitCat)