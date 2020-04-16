project(JitCatUnitTests)

include_directories(
    ${JitCatIncludePath}
	${DependenciesIncludePath}
	${JitCatUnitTestsIncludePath}
)

file(GLOB JitCatUnitTestsHeaderFiles ${JitCatUnitTestsIncludePath}/*.h)

if (JitCatUnitTestsHeaderFiles)
	set_source_files_properties(${JitCatUnitTestsHeaderFiles} PROPERTIES HEADER_FILE_ONLY ON)
endif()

set(Source_GeneralTests
	AssignmentTests.cpp
	BasicTypesTests.cpp
	ContainerTests.cpp
	CustomTypesTests.cpp
	ExpressionAnyTests.cpp
	IndirectionTests.cpp
	MemberFunctionCallTests.cpp
	OperatorPrecedenceTests.cpp
	OperatorOverloadingTests.cpp
	StaticFunctionCallTests.cpp
)

set(Source_BuiltInFunctionTests
	MathsFunctionsTests.cpp
	RandomFunctionsTests.cpp
	SelectFunctionTests.cpp
	StringFunctionsTests.cpp
	TrigonometricFunctionsTests.cpp
	TypeConversionFunctionTests.cpp
)

set(Source_Helper
	TestObjects.cpp
	TestObjects.h
	TestHelperFunctions.h
)

set(Source_RegressionTests
	RegressionTests.cpp
)

set(Source_Root
	main.cpp
	Tests.cpp
)
	
source_group(GeneralTests FILES	${Source_GeneralTests})
source_group(BuiltInFunctionTests FILES	${Source_BuiltInFunctionTests})
source_group(RegressionTests FILES	${Source_RegressionTests})
source_group(Helper FILES	${Source_Helper})
source_group("" FILES ${Source_Root})

add_executable(JitCatUnitTests 
	${Source_GeneralTests}
	${Source_BuiltInFunctionTests}
	${Source_RegressionTests}
	${Source_Helper}
	${Source_Root}
)
	
add_dependencies(JitCatUnitTests JitCat)

target_link_libraries(JitCatUnitTests JitCat)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER JitCat)