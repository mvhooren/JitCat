project(JitCatTest)

include_directories(
    ../../../include/jitcat
	../../../test/jitcattest
)

file(GLOB JITCATTEST_SOURCE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ../../../test/jitcattest/*.h
    ../../../test/jitcattest/*.cpp
)

add_executable(JitCatTest ${JITCATTEST_SOURCE})
add_dependencies(JitCatTest JitCat)

link_directories(${CMAKE_SOURCE_DIR}/../../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})
target_link_libraries(JitCatTest JitCat)

set_target_properties(JitCatTest
    PROPERTIES
    COMPILE_FLAGS "-std=c++11 -Wno-write-strings -Wno-switch -Wno-unused-function"
    LINK_FLAGS_RELEASE "-Wl,-s,-x"
)

install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/../../bin/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})