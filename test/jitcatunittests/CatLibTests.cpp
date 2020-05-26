/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatLib.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Configuration.h"
#include "jitcat/TypeInfo.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


TEST_CASE("CatLib Tests", "[.catlib]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	CatLib library("TestLib");
	library.addStaticScope(&reflectedObject);

	library.addSource("test1.jc", 
		"class TestClass\n"
		"{\n"
		"	float testFloat = 42.0f;\n"
		"	double testDouble = 84.0;\n"
		"	int testInt = -1;\n"
		"	bool testBool = true;\n"
		"	string testString = \"Hello!\";\n"
		"\n"
		"	float getFloat() { return testFloat;}\n"
		"	double getDouble() { return testDouble;}\n"
		"	int getInt() { return testInt;}\n"
		"	bool getBool() { return testBool;}\n"
		"	string getString() { return testString;}\n"
		"}\n"
		);
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
	context.addScope(testClassInfo, testClassInstance, false);

	SECTION("Float variable")
	{
		Expression<float> testExpression(&context, "testFloat");
		doChecks(42.0f, false, false, false, testExpression, context);
	}
	SECTION("Float function")
	{
		Expression<float> testExpression(&context, "getFloat()");
		doChecks(42.0f, false, false, false, testExpression, context);
	}

	SECTION("Double variable")
	{
		Expression<double> testExpression(&context, "testDouble");
		doChecks(84.0, false, false, false, testExpression, context);
	}
	SECTION("Double function")
	{
		Expression<double> testExpression(&context, "getDouble()");
		doChecks(84.0, false, false, false, testExpression, context);
	}

	SECTION("Int variable")
	{
		Expression<int> testExpression(&context, "testInt");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("Int function")
	{
		Expression<int> testExpression(&context, "getInt()");
		doChecks(-1, false, false, false, testExpression, context);
	}

	SECTION("Bool variable")
	{
		Expression<bool> testExpression(&context, "testBool");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Bool function")
	{
		Expression<bool> testExpression(&context, "getBool()");
		doChecks(true, false, false, false, testExpression, context);
	}

	SECTION("String variable")
	{
		Expression<std::string> testExpression(&context, "testString");
		doChecks(std::string("Hello!"), false, false, false, testExpression, context);
	}
	SECTION("String function")
	{
		Expression<std::string> testExpression(&context, "getString()");
		doChecks(std::string("Hello!"), false, false, false, testExpression, context);
	}

	testClassInfo->destruct(testClassInstance);
}
