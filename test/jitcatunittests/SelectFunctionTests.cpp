/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/TypeInfo.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


TEST_CASE("Builtin functions test: Select", "[builtins][select]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Select", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Select_cc1")
	{
		Expression<int> testExpression(&context, "select(true, 1, 3)");
		doChecks(1, false, true, false, testExpression, context);
	}
	SECTION("Select_cc2")
	{
		Expression<float> testExpression(&context, "select(false, 11.0f, 12)");
		doChecks(12.0f, false, true, false, testExpression, context);
	}
	SECTION("Select_cc3")
	{
		Expression<std::string> testExpression(&context, "select(true, \"test\", \"bla\")");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("Select_cc4")
	{
		Expression<bool> testExpression(&context, "select(false, false, true)");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Select_float1")
	{
		Expression<int> testExpression(&context, "select(aFloat > 900, 4, 5)");
		doChecks(4, false, false, false, testExpression, context);
	}
	SECTION("Select_float2")
	{
		Expression<std::string> testExpression(&context, "select(aFloat != 0, \"test\", 1.0f)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("Select_int1")
	{
		Expression<int> testExpression(&context, "select(largeInt == 1234567, -7, 789)");
		doChecks(-7, false, false, false, testExpression, context);
	}
	SECTION("Select_int2")
	{
		Expression<int> testExpression(&context, "select(aBoolean, -largeInt, largeInt)");
		doChecks(-1234567, false, false, false, testExpression, context);
	}
	SECTION("Select_int3")
	{
		Expression<int> testExpression(&context, "select(largeInt != 0, largeInt, largeInt)");
		doChecks(1234567, false, false, false, testExpression, context);
	}
	SECTION("Select_bool")
	{
		Expression<std::string> testExpression(&context, "select(!aBoolean, \"0\", \"1\")");
		doChecks(std::string("1"), false, false, false, testExpression, context);
	}
	SECTION("Select_string1")
	{
		Expression<std::string> testExpression(&context, "select(stringLength(\"10\") > 0, text, numberString)");
		doChecks(std::string("Hello!"), false, false, false, testExpression, context);
	}
	SECTION("Select_string2")
	{
		Expression<float> testExpression(&context, "select(numberString == \"\", 3.0f, 2.0f)");
		doChecks(2.0f, false, false, false, testExpression, context);
	}
	SECTION("Select_string3")
	{
		Expression<std::string> testExpression(&context, "select(text == numberString, text, numberString)");
		doChecks(std::string("123.4"), false, false, false, testExpression, context);
	}
	SECTION("Select_noarg")
	{
		Expression<std::string> testExpression(&context, "select()");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("Select_morearg")
	{
		Expression<std::string> testExpression(&context, "select(aBoolean, aFloat, 10, largeInt)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("Select_obj")
	{
		Expression<std::string> testExpression(&context, "select(nestedObject, 10, 11)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
}
