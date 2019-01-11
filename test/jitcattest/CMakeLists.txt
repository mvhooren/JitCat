project(JitCatTest)

include_directories(
    ${JitCatIncludePath}
	../../test/jitcattest
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
link_directories(${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})
target_link_libraries(JitCatTest 
					  JitCat)
					  
#Set C++ 17 standard					  
set_property(TARGET JitCatTest PROPERTY CXX_STANDARD 17)			  

if(UNIX)			  
set_target_properties(JitCatTest
    PROPERTIES
    LINK_FLAGS_RELEASE "-Wl,-s,-x"
)
endif()
