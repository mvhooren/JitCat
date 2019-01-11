project(JitCatValidator)

include_directories(
    ${JitCatIncludePath}
	${JitCatValidatorIncludePath}
)

set(JitCatValidatorSourceFiles
	JitCatValidator.cpp
	dllmain.cpp
)

set(JitCatValidatorHeaderFiles
	${JitCatValidatorIncludePath}/JitCatValidator.h
)
	

set_source_files_properties(${JitCatValidatorHeaderFiles} PROPERTIES HEADER_FILE_ONLY ON)

if(NOT MSVC)
	add_compile_options(-fPIC)
endif()

add_library(JitCatValidator SHARED 
	${JitCatValidatorSourceFiles}
	${JitCatValidatorHeaderFiles}
)
	
add_dependencies(JitCatValidator JitCat)

source_group("" FILES 	
	${JitCatValidatorSourceFiles}
	${JitCatValidatorHeaderFiles}
)
	

add_compile_definitions(JITCATVALIDATOR_EXPORTS)

link_directories(${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})
target_link_libraries(JitCatValidator 
					  PRIVATE JitCat)
					  
#Set C++ 17 standard
set_property(TARGET JitCatValidator PROPERTY CXX_STANDARD 17)

if(UNIX)
set_target_properties(JitCatValidator
    PROPERTIES
    LINK_FLAGS_RELEASE "-Wl,-s,-x"
)
endif()
