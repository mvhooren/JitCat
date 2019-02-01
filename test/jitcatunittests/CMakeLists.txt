project(JitCatUnitTests)

include_directories(
    ${JitCatIncludePath}
	${DependenciesIncludePath}
	${JitCatUnitTestsIncludePath}
)


set(JitCatUnitTestsSourceFiles
	TestObjects.cpp
	main.cpp
	Tests.cpp
)

set(JitCatUnitTestsHeaderFiles TestObjects.h)
	
set_source_files_properties(${JitCatUnitTestsHeaderFiles} PROPERTIES HEADER_FILE_ONLY ON)

source_group("" FILES 	
	${JitCatUnitTestsSourceFiles}
	${JitCatUnitTestsHeaderFiles}
)

add_executable(JitCatUnitTests 
	${JitCatUnitTestsSourceFiles}
	${JitCatUnitTestsHeaderFiles}
)
	
add_dependencies(JitCatUnitTests JitCat)

target_link_libraries(JitCatUnitTests JitCat)
