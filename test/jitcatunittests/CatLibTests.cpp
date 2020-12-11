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

//Tests simple variable initialization and access.
TEST_CASE("CatLib basic tests", "[catlib]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	CatLib library("TestLib");
	library.addStaticScope(&reflectedObject);
	Tokenizer::Document source(
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


//Tests overloaded funcion functionality.
TEST_CASE("CatLib function overloading tests", "[catlib][function_overloading]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	CatLib library("TestLib");
	library.addStaticScope(&reflectedObject);

	Tokenizer::Document source(
		"class TestClass\n"
		"{\n"
		"	string testString = \"Hello!\";\n"
		"\n"
		"	string addToTestString(int value) { return testString + value;}\n"
		"	string addToTestString(string value) { return testString + value;}\n"
		"}\n"
		"class DuplicateTestClass\n"
		"{\n"
		"	string testString = \"Hello2!\";\n"
		"\n"
		"	string addToTestString(int value) { return testString + value * 2;}\n"
		"	string addToTestString(string value) { return testString + value + value;}\n"
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

	TypeInfo* duplicateTestClassInfo = library.getTypeInfo("DuplicateTestClass");
	REQUIRE(duplicateTestClassInfo != nullptr);
	unsigned char* duplicateTestClassInstance = duplicateTestClassInfo->construct();

	CatRuntimeContext context("jitlib", &errorManager);
	context.addScope(testClassInfo, testClassInstance, false);

	CatRuntimeContext alternativeContext("jitlib_alternative", &errorManager);
	alternativeContext.addScope(duplicateTestClassInfo, duplicateTestClassInstance, false);

	SECTION("Test function")
	{
		Expression<std::string> testExpression1(&context, "addToTestString(42)");
		doChecks(std::string("Hello!42"), false, false, false, testExpression1, context);

		Expression<std::string> testExpression2(&context, "addToTestString(\" World!\")");
		doChecks(std::string("Hello! World!"), false, false, false, testExpression2, context);

		Expression<std::string> testExpression3(&alternativeContext, "addToTestString(42)");
		doChecks(std::string("Hello2!84"), false, false, false, testExpression3, alternativeContext);

		Expression<std::string> testExpression4(&alternativeContext, "addToTestString(\" World!\")");
		doChecks(std::string("Hello2! World! World!"), false, false, false, testExpression4, alternativeContext);
	}


	testClassInfo->destruct(testClassInstance);
	duplicateTestClassInfo->destruct(duplicateTestClassInstance);
}


//Tests local variable functionality.
TEST_CASE("CatLib local variable tests", "[catlib][locals]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	CatLib library("TestLib");
	library.addStaticScope(&reflectedObject);

	Tokenizer::Document source(		
		"class TestClass\n"
		"{\n"
		"	string testString = \"Hello!\";\n"
		"\n"
		"	string addToTestString(int value)\n"
		"	{\n"
		"		string tempString = testString + value;\n"
		"		string tempString2 = value + tempString;\n"
		"		return tempString2\n;"
		"	}\n"
		"\n"
		"	float checkFloats(float a, float b)\n"
		"	{\n"
		"		//Intentionally named tempString to check if there are no naming conflicts with locals defined in addToTestString. \n"
		"		float tempString = 11.0f;\n"
		"		float tempString2 = tempString * a;\n"
		"		return tempString + tempString2 * b;\n"
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
	context.addScope(testClassInfo, testClassInstance, false);

	SECTION("Local tests")
	{
		Expression<std::string> testExpression1(&context, "addToTestString(42)");
		doChecks(std::string("42Hello!42"), false, false, false, testExpression1, context);

		Expression<float> testExpression2(&context, "checkFloats(12.0f, 91.0f)");
		doChecks(12023.0f, false, false, false, testExpression2, context);
	}

	testClassInfo->destruct(testClassInstance);
}


//Tests if-statement functionality.
TEST_CASE("CatLib if statement tests", "[catlib][if-statement][control-flow]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	CatLib library("TestLib");
	library.addStaticScope(&reflectedObject);

	Tokenizer::Document source(
		"class TestClass\n"
		"{\n"
		"	string testString = \"Hello!\";\n"
		"	string testString2 = \"World!\";\n"
		"	string testString3 = \"Hello world!\";\n"
		"\n"
		"	string basicIf(int value)\n"
		"	{\n"
		"		if (value > 0)\n"
		"		{\n"
		"			return testString;\n"
		"		}\n"
		"		else\n"
		"		{\n"
		"			return testString2;\n"
		"		}\n"
		"	}\n"
		"\n"
		"	string fallThroughIf(int value)\n"
		"	{\n"
		"		if (value > 0)\n"
		"		{\n"
		"			return testString;\n"
		"		}\n"
		"		return testString2;\n"
		"	}\n"
		"\n"
		"	string basicElseIf(int value)\n"
		"	{\n"
		"		if (value > 0)\n"
		"		{\n"
		"			return testString;\n"
		"		}\n"
		"		else if (value == 0)\n"
		"		{\n"
		"			return testString2;\n"
		"		}\n"
		"		else\n"
		"		{\n"
		"			return testString3;\n"
		"		}\n"
		"	}\n"
		"\n"
		"	string nestedElseIf(int value)\n"
		"	{\n"
		"		if (value > 0)\n"
		"		{\n"
		"			return testString;\n"
		"		}\n"
		"		else\n"
		"		{\n"
		"			if (value == 0)\n"
		"			{\n"
		"				return testString2;\n"
		"			}\n"
		"			else\n"
		"			{\n"
		"				return testString3;\n"
		"			}\n"
		"		}\n"
		"	}\n"
		"\n"
		"	string nestedElseIf2(int value)\n"
		"	{\n"
		"		if (value <= 0)\n"
		"		{\n"
		"			if (value == 0)\n"
		"			{\n"
		"				return testString2;\n"
		"			}\n"
		"			else\n"
		"			{\n"
		"				return testString3;\n"
		"			}\n"
		"		}\n"
		"		else\n"
		"		{\n"
		"			return testString;\n"
		"		}\n"
		"	}\n"
		"\n"
		"	int floatIf(float value)\n"
		"	{\n"
		"		int result = 0;\n"
		"		if (value > 0.0f)\n"
		"		{\n"
		"			result = result + 1;\n"
		"		}\n"
		"		if (value > 1.0f)\n"
		"		{\n"
		"			result = result + 1;\n"
		"		}\n"
		"		return result;\n"
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
	context.addScope(testClassInfo, testClassInstance, false);

	SECTION("If tests")
	{
		Expression<std::string> testExpression1(&context, "basicIf(42)");
		doChecks(std::string("Hello!"), false, false, false, testExpression1, context);

		Expression<std::string> testExpression2(&context, "basicIf(-1)");
		doChecks(std::string("World!"), false, false, false, testExpression2, context);

		Expression<std::string> testExpression3(&context, "fallThroughIf(42)");
		doChecks(std::string("Hello!"), false, false, false, testExpression3, context);

		Expression<std::string> testExpression4(&context, "fallThroughIf(-1)");
		doChecks(std::string("World!"), false, false, false, testExpression4, context);

		Expression<std::string> testExpression5(&context, "basicElseIf(42)");
		doChecks(std::string("Hello!"), false, false, false, testExpression5, context);

		Expression<std::string> testExpression6(&context, "basicElseIf(-1)");
		doChecks(std::string("Hello world!"), false, false, false, testExpression6, context);

		Expression<std::string> testExpression7(&context, "basicElseIf(0)");
		doChecks(std::string("World!"), false, false, false, testExpression7, context);

		Expression<std::string> testExpression8(&context, "nestedElseIf(42)");
		doChecks(std::string("Hello!"), false, false, false, testExpression8, context);

		Expression<std::string> testExpression9(&context, "nestedElseIf(-1)");
		doChecks(std::string("Hello world!"), false, false, false, testExpression9, context);

		Expression<std::string> testExpression10(&context, "nestedElseIf(0)");
		doChecks(std::string("World!"), false, false, false, testExpression10, context);

		Expression<std::string> testExpression11(&context, "nestedElseIf2(42)");
		doChecks(std::string("Hello!"), false, false, false, testExpression11, context);

		Expression<std::string> testExpression12(&context, "nestedElseIf2(-1)");
		doChecks(std::string("Hello world!"), false, false, false, testExpression12, context);

		Expression<std::string> testExpression13(&context, "nestedElseIf2(0)");
		doChecks(std::string("World!"), false, false, false, testExpression13, context);

		Expression<int> testExpression14(&context, "floatIf(-1.0f)");
		doChecks(0, false, false, false, testExpression14, context);

		Expression<int> testExpression15(&context, "floatIf(0.1f)");
		doChecks(1, false, false, false, testExpression15, context);

		Expression<int> testExpression16(&context, "floatIf(11.0f)");
		doChecks(2, false, false, false, testExpression16, context);
	}

	testClassInfo->destruct(testClassInstance);
}


//Tests for-loop functionality.
TEST_CASE("CatLib for loop tests", "[catlib][for-loop][control-flow]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	CatLib library("TestLib");
	library.addStaticScope(&reflectedObject);

	Tokenizer::Document source(
		"class TestClass\n"
		"{\n"
		"	string testString = \"Hello!\";\n"
		"\n"
		"	int simpleFor(int value)\n"
		"	{\n"
		"		int total = 0;\n"
		"		for i in range (value)\n"
		"		{\n"
		"			total = total + i * i;\n"
		"		}\n"
		"		return total;\n"
		"	}\n"
		"\n"
		"	int simpleFor2(int value)\n"
		"	{\n"
		"		int total = 0;\n"
		"		for i in range (0, value)\n"
		"		{\n"
		"			total = total + i * i;\n"
		"		}\n"
		"		return total;\n"
		"	}\n"
		"\n"
		"	int nestedFor(int value, int value2)\n"
		"	{\n"
		"		int total = 0;\n"
		"		for i in range (0, value)\n"
		"		{\n"
		"			for j in range (0, value2)\n"
		"			{\n"
		"				total = total + 1;\n"
		"			}\n"
		"		}\n"
		"		return total;\n"
		"	}\n"
		"\n"
		"	int earlyOutFor(int value)\n"
		"	{\n"
		"		int total = 0;\n"
		"		for i in range (value)\n"
		"		{\n"
		"			return total + 42;\n"
		"		}\n"
		"		return total;\n"
		"	}\n"
		"\n"
		"	int stepFor(int value)\n"
		"	{\n"
		"		int total = 0;\n"
		"		for i in range (0, value, 2)\n"
		"		{\n"
		"			total = total + 1;\n"
		"		}\n"
		"		return total;\n"
		"	}\n"
		"\n"
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
	context.addScope(testClassInfo, testClassInstance, false);

	SECTION("Local tests")
	{
		{
			Expression<int> testExpression(&context, "simpleFor(4)");
			doChecks(14, false, false, false, testExpression, context);
		}{
			Expression<int> testExpression(&context, "simpleFor(0)");
			doChecks(0, false, false, false, testExpression, context);
		}{
			Expression<int> testExpression(&context, "simpleFor2(5)");
			doChecks(30, false, false, false, testExpression, context);
		}{
			Expression<int> testExpression(&context, "nestedFor(5, 11)");
			doChecks(55, false, false, false, testExpression, context);
		}{
			Expression<int> testExpression(&context, "earlyOutFor(5)");
			doChecks(42, false, false, false, testExpression, context);
		}{
			Expression<int> testExpression(&context, "earlyOutFor(0)");
			doChecks(0, false, false, false, testExpression, context);
		}{
			Expression<int> testExpression(&context, "stepFor(0)");
			doChecks(0, false, false, false, testExpression, context);
		}{
			Expression<int> testExpression(&context, "stepFor(2)");
			doChecks(1, false, false, false, testExpression, context);
		}{
			Expression<int> testExpression(&context, "stepFor(3)");
			doChecks(2, false, false, false, testExpression, context);
		}{
			Expression<int> testExpression(&context, "stepFor(4)");
			doChecks(2, false, false, false, testExpression, context);
		}
	}

	testClassInfo->destruct(testClassInstance);
}


TEST_CASE("CatLib local function call tests", "[catlib][local_function_call]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	CatLib library("TestLib");
	library.addStaticScope(&reflectedObject);

	Tokenizer::Document source(		
		"class TestClass\n"
		"{\n"
		"\n"
		"	float addToFloat(int value)\n"
		"	{\n"
		"		return getMyFloat() + value\n;"
		"	}\n"
		"\n"
		"	float getMyFloat()\n"
		"	{\n"
		"		return myFloat;\n"
		"	}\n"
		"\n"
		"	float myFloat = 42.0f;\n"
		"}\n");

	library.addSource("test1.jc", source);
	std::vector<const ExpressionErrorManager::Error*> errors;
	library.getErrorManager().getAllErrors(errors);
	for (auto& iter : errors)
	{
		std::cout << iter->contextName << " ERROR: Line: " << iter->errorLine + 1 << " Column: " << iter->errorColumn << " Length: " << iter->errorLength << "\n";
		std::cout << iter->message << "\n";
	}
	REQUIRE(library.getErrorManager().getNumErrors() == 0);
	TypeInfo* testClassInfo = library.getTypeInfo("TestClass");
	REQUIRE(testClassInfo != nullptr);
	unsigned char* testClassInstance = testClassInfo->construct();

	CatRuntimeContext context("jitlib", &errorManager);
	context.addScope(testClassInfo, testClassInstance, false);

	SECTION("Local tests")
	{
		Expression<float> testExpression1(&context, "addToFloat(43)");
		doChecks(85.0f, false, false, false, testExpression1, context);
	}

	testClassInfo->destruct(testClassInstance);
}


TEST_CASE("CatLib use before defined", "[catlib][use_before_defined]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	CatLib library("TestLib");
	library.addStaticScope(&reflectedObject);

	Tokenizer::Document source(		
		"class TestClass\n"
		"{\n"
		"	VectorClass memberVector;\n"
		"\n"
		"	VectorClass addToVector(int value)\n"
		"	{\n"
		"		VectorClass test;\n"
		"		test.x = test.x + value;\n"
		"		return test;\n"
		"	}\n"
		"\n"
		"	float getX()\n"
		"	{\n"
		"		return addToVector(42).x;\n"
		"	}\n"
		"\n"
		"	float getY()\n"
		"	{\n"
		"		return memberVector.y + 42;\n"
		"	}\n"
		"}\n"
		"\n"
		"class VectorClass\n"
		"{\n"
		"	float x = 1.0f;\n"
		"	float y = 2.0f;\n"
		"	float z = 3.0f;\n"
		"	float w = 4.0f;\n"
		"}\n"	
	);

	library.addSource("test1.jc", source);
	std::vector<const ExpressionErrorManager::Error*> errors;
	library.getErrorManager().getAllErrors(errors);
	for (auto& iter : errors)
	{
		std::cout << iter->contextName << " ERROR: Line: " << iter->errorLine + 1 << " Column: " << iter->errorColumn << " Length: " << iter->errorLength << "\n";
		std::cout << iter->message << "\n";
	}
	REQUIRE(library.getErrorManager().getNumErrors() == 0);
	TypeInfo* testClassInfo = library.getTypeInfo("TestClass");
	REQUIRE(testClassInfo != nullptr);
	unsigned char* testClassInstance = testClassInfo->construct();

	CatRuntimeContext context("jitlib", &errorManager);
	context.addScope(testClassInfo, testClassInstance, false);

	SECTION("getX")
	{
		Expression<float> testExpression1(&context, "getX()");
		doChecks(43.0f, false, false, false, testExpression1, context);
	}
	SECTION("getY")
	{
		Expression<float> testExpression1(&context, "getY()");
		doChecks(44.0f, false, false, false, testExpression1, context);
	}

	testClassInfo->destruct(testClassInstance);
}


TEST_CASE("CatLib inheritance", "[.][catlib][inheritance]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	CatLib library("TestLib");
	library.addStaticScope(&reflectedObject);

	Tokenizer::Document source(		
		"class TestClass\n"
		"{\n"
		"	inherits VectorClass;\n"
		"	inherits AnotherClass;\n"
		"\n"
		"	float getX()\n"
		"	{\n"
		"		return x + 1;\n"
		"	}\n"
		"\n"
		"	float getY()\n"
		"	{\n"
		"		return y + 42;\n"
		"	}\n"
		"\n"
		"	float getW()\n"
		"	{\n"
		"		return vector.w + 11;\n"
		"	}\n"
		"}\n"
		"\n"
		"class VectorClass\n"
		"{\n"
		"	float x = 1.0f;\n"
		"	float y = 2.0f;\n"
		"	float z = 3.0f;\n"
		"	float w = 4.0f;\n"
		"}\n"	
		"\n"
		"class AnotherClass\n"
		"{\n"
		"	VectorClass vector;\n"
		"}\n"
	);

	library.addSource("test1.jc", source);
	std::vector<const ExpressionErrorManager::Error*> errors;
	library.getErrorManager().getAllErrors(errors);
	for (auto& iter : errors)
	{
		std::cout << iter->contextName << " ERROR: Line: " << iter->errorLine + 1 << " Column: " << iter->errorColumn << " Length: " << iter->errorLength << "\n";
		std::cout << iter->message << "\n";
	}
	REQUIRE(library.getErrorManager().getNumErrors() == 0);
	TypeInfo* testClassInfo = library.getTypeInfo("TestClass");
	REQUIRE(testClassInfo != nullptr);
	unsigned char* testClassInstance = testClassInfo->construct();

	CatRuntimeContext context("jitlib", &errorManager);
	context.addScope(testClassInfo, testClassInstance, false);

	SECTION("getX")
	{
		Expression<float> testExpression1(&context, "getX()");
		doChecks(2.0f, false, false, false, testExpression1, context);
	}
	SECTION("getY")
	{
		Expression<float> testExpression1(&context, "getY()");
		doChecks(44.0f, false, false, false, testExpression1, context);
	}
	SECTION("getW")
	{
		Expression<float> testExpression1(&context, "getW()");
		doChecks(15.0f, false, false, false, testExpression1, context);
	}

	testClassInfo->destruct(testClassInstance);
}


//Expected to fail for now
TEST_CASE("CatLib arrays", "[.][catlib][arrays]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	CatLib library("TestLib");
	library.addStaticScope(&reflectedObject);

	Tokenizer::Document source(		
		"class TestClass\n"
		"{\n"
		"	float[] floats;\n"
		"\n"
		"	void init()\n"
		"	{\n"
		"		floats.add(12.34f);\n"
		"	}\n"
		"	float getFloat()\n"
		"	{\n"
		"		return floats[0];\n"
		"	}\n"
		"}\n"

	);

	library.addSource("test1.jc", source);
	std::vector<const ExpressionErrorManager::Error*> errors;
	library.getErrorManager().getAllErrors(errors);
	for (auto& iter : errors)
	{
		std::cout << iter->contextName << " ERROR: Line: " << iter->errorLine + 1 << " Column: " << iter->errorColumn << " Length: " << iter->errorLength << "\n";
		std::cout << iter->message << "\n";
	}
	REQUIRE(library.getErrorManager().getNumErrors() == 0);
	TypeInfo* testClassInfo = library.getTypeInfo("TestClass");
	REQUIRE(testClassInfo != nullptr);
	unsigned char* testClassInstance = testClassInfo->construct();

	CatRuntimeContext context("jitlib", &errorManager);
	context.addScope(testClassInfo, testClassInstance, false);

	SECTION("getX")
	{
		Expression<float> testExpression1(&context, "getFloat()");
		doChecks(12.34f, false, false, false, testExpression1, context);
	}

	testClassInfo->destruct(testClassInstance);
}