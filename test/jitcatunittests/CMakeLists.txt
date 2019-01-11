project(JitCatUnitTests)

include_directories(
    ${JitCatIncludePath}
	${DependenciesIncludePath}
	../../test/jitcatunittests
)


set(JitCatUnitTestsSourceFiles
	TestObjects.cpp
	main.cpp
	Tests.cpp
)

set(JitCatUnitTestsHeaderFiles
	TestObjects.h
)
	
set_source_files_properties(${JitCatUnitTestsHeaderFiles} PROPERTIES HEADER_FILE_ONLY ON)

source_group("" FILES 	
	${JitCatUnitTestsSourceFiles}
	${JitCatUnitTestsHeaderFiles}
)

add_executable(JitCatUnitTests 
	${JitCatUnitTestsSourceFiles}
	${JitCatUnitTestsHeaderFiles})
	
add_dependencies(JitCatUnitTests JitCat)

link_directories(${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})
target_link_libraries(JitCatUnitTests 
					  JitCat)
					  
#Set C++ 17 standard					  
set_property(TARGET JitCatUnitTests PROPERTY CXX_STANDARD 17)
					  
if(UNIX)
set_target_properties(JitCatUnitTests
    PROPERTIES
    LINK_FLAGS_RELEASE "-Wl,-s,-x"
)
endif()
