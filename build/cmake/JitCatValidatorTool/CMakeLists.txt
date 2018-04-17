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
target_link_libraries(JitCatValidatorTool JitCatValidator)

set_target_properties(JitCatValidatorTool
    PROPERTIES
    COMPILE_FLAGS "-std=c++11 -Wno-write-strings -Wno-unused-function"
    LINK_FLAGS_RELEASE "-Wl,-s,-x"
	INSTALL_RPATH "./"
)

install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/../../bin/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})