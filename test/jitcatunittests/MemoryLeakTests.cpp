/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatLib.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/TypeInfo.h"
#include "PrecompilationTest.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


TEST_CASE("Basic memory leak test", "[memory]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("memberFunctions", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "memoryStaticScope");	


	SECTION("Get TestVector4")
	{
		int currentInstances = TestVector4::instanceCount;
		{
			Expression<TestVector4> testExpression(&context, "getTestVector()");
			doChecks(reflectedObject.getTestVector(), false, false, false, testExpression, context);
		}
		CHECK(currentInstances == TestVector4::instanceCount);
	}
	SECTION("Add TestVector4")
	{
		int currentInstances = TestVector4::instanceCount;
		{
			Expression<TestVector4> testExpression(&context, "addVectors(getTestVector(), getTestVectorPtr())");
			doChecks(reflectedObject.addVectors(reflectedObject.getTestVector(), *reflectedObject.getTestVectorPtr()), false, false, false, testExpression, context);
		}
		CHECK(currentInstances == TestVector4::instanceCount);
	}
}


//Tests memory leaks in catlib code.
TEST_CASE("CatLib memory leak tests", "[catlib][memory]" ) 
{
	bool enableTest = !Configuration::usePreCompiledExpressions && Precompilation::precompContext == nullptr;

	if (!enableTest)
	{
		if (Configuration::usePreCompiledExpressions )
		{
			WARN("CatLib tests are disabled because Configuration::usePreCompiledExpressions is enabled and CatLib does not yet support precompilation");
		}
		else
		{
			WARN("CatLib tests are disabled because there is an active precompilation context and CatLib does not yet support precompilation");
		}
	}
	if (enableTest)
	{
		ReflectedObject reflectedObject;
		ExpressionErrorManager errorManager;

		CatLib library("TestLib", Precompilation::precompContext);
		library.addStaticScope(&reflectedObject, "catlibMemoryStaticScope");
		Tokenizer::Document source(
			"class TestClass\n"
			"{\n"
			"	//This calls the getTestVector from reflectedObject.\n"
			"	TestVector4 testVector = getTestVector();\n"
			"\n"
			"	TestVector4 getTestVector2()\n"
			"	{\n"
			"		return testVector;\n"
			"	}\n"
			"\n"
			"	TestVector4 addTestVector(TestVector4 vectorToAdd)\n"
			"	{\n"
			"		TestVector4 result = vectorToadd + testVector;\n"
			"		return result;\n"
			"	}\n"
			"\n"
			"	TestVector4 checkTestVector(TestVector4 vectorToCheck)\n"
			"	{\n"
			"		if (vectorToCheck.x > 42.0f)\n"
			"		{\n"
			"			TestVector4 result = vectorToCheck + testVector;\n"
			"			return result;\n"
			"		}\n"
			"		else\n"
			"		{\n"
			"			TestVector4 result = vectorToCheck - testVector;\n"
			"			return result;\n"
			"		}\n"
			"	}\n"
			"}\n");
		library.addSource("test1.jc", source);
		std::vector<const ExpressionErrorManager::Error*> errors;
		library.getErrorManager().getAllErrors(errors);
		for (auto& iter : errors)
		{
			std::cout << iter->contextName << " ERROR: Line: " << iter->errorLine << " Column: " << iter->errorColumn << " Length: " << iter->errorLength << "\n";
			std::cout << iter->message << "\n";
		}
		REQUIRE(library.getErrorManager().getNumErrors() == 0);
		TypeInfo* testClassInfo = library.getTypeInfo("TestClass");
		REQUIRE(testClassInfo != nullptr);
		unsigned char* testClassInstance = testClassInfo->construct();

		CatRuntimeContext context("jitlib", &errorManager);
		context.setPrecompilationContext(Precompilation::precompContext);
		context.addDynamicScope(testClassInfo, testClassInstance);

		SECTION("Get TestVector4")
		{
			int currentInstances = TestVector4::instanceCount;
			{
				Expression<TestVector4> testExpression1(&context, "getTestVector2()");
				doChecks(TestVector4(1.0f, 2.0f, 3.0f, 4.0f), false, false, false, testExpression1, context);
			}
			CHECK(currentInstances == TestVector4::instanceCount);
		}

		SECTION("Add TestVector4")
		{
			int currentInstances = TestVector4::instanceCount;
			{
				Expression<TestVector4> testExpression1(&context, "addTestVector(getTestVector2())");
				doChecks(TestVector4(2.0f, 4.0f, 6.0f, 8.0f), false, false, false, testExpression1, context);
			}
			CHECK(currentInstances == TestVector4::instanceCount);
		}

		SECTION("Check TestVector4")
		{
			int currentInstances = TestVector4::instanceCount;
			{
				Expression<TestVector4> testExpression1(&context, "checkTestVector(getTestVector2())");
				doChecks(TestVector4(0.0f, 0.0f, 0.0f, 0.0f), false, false, false, testExpression1, context);
			}
			CHECK(currentInstances == TestVector4::instanceCount);
		}
	}
}