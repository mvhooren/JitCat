project(JitCatValidator)

include_directories(${JitCatIncludePath})

set(JitCatValidatorSourceFiles
	JitCatValidator.cpp
	dllmain.cpp
)

set(JitCatValidatorHeaderFiles	${JitCatValidatorIncludePath}/jitcatvalidator/JitCatValidator.h)
	

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

target_link_libraries(JitCatValidator PRIVATE JitCat)
					  