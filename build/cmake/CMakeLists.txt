cmake_minimum_required(VERSION 3.5)
project(JitCat)

option(FORCE32 "Force a 32bit compile on 64bit" OFF)

if (NOT CMAKE_BUILD_TYPE)
 set(CMAKE_BUILD_TYPE "Release")
endif ()


if(FORCE32)
    set(CMAKE_C_FLAGS "-m32")
    set(CMAKE_CXX_FLAGS "-m32 -fpermissive")
endif()

if(CMAKE_SIZEOF_VOID_P MATCHES "8" AND NOT(FORCE32))
	set(JITCAT_BITNESS "64")
else()
	set(JITCAT_BITNESS "32")
endif()

include_directories(
    ../../include/jitcat
)

file(GLOB JITCAT_SOURCE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ../../include/jitcat/*.h
    ../../src/jitcat/*.cpp
)

add_library(JitCat STATIC ${JITCAT_SOURCE})

set_target_properties(JitCat
    PROPERTIES
    COMPILE_FLAGS "-std=c++11 -Wno-write-strings -Wno-switch -Wno-unused-function -fPIC"
    LINK_FLAGS_RELEASE "-Wl,-s,-x"
)

add_subdirectory(JitCatTest)
add_subdirectory(JitCatValidator)
add_subdirectory(JitCatValidatorTool)

install(TARGETS ${PROJECT_NAME}
		RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/../../bin/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		LIBRARY DESTINATION ${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE}
		ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/../../lib/${CMAKE_SYSTEM_NAME}${JITCAT_BITNESS}/${CMAKE_BUILD_TYPE})
		
		