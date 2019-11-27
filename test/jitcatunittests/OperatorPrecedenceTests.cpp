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


TEST_CASE("Operator precedence", "[operators][precedence]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Select", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Precedence test 1")
	{
		Expression<int> testExpression(&context, "3 + 5 * 11 - 8 / 2");
		doChecks(3 + 5 * 11 - 8 / 2, false, true, false, testExpression, context);
	}
	SECTION("Precedence test 2")
	{
		Expression<bool> testExpression(&context, "false && false || true");
		doChecks(false && false || true, false, true, false, testExpression, context);
	}
	SECTION("Precedence test 3")
	{
		Expression<int> testExpression(&context, "theInt + theInt * largeInt - theInt / 2");
		doChecks(reflectedObject.theInt + reflectedObject.theInt * reflectedObject.largeInt - reflectedObject.theInt / 2, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 4")
	{
		Expression<bool> testExpression(&context, "no && no || aBoolean");
		doChecks(false && false || true, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 5")
	{
		Expression<bool> testExpression(&context, "aBoolean != no || aBoolean");
		doChecks(true != false || true, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 5")
	{
		Expression<bool> testExpression(&context, "aBoolean == no || no");
		doChecks(true == false || false, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 6")
	{
		Expression<bool> testExpression(&context, "theInt < largeInt || no");
		doChecks(42 < 100 || false, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 6")
	{
		Expression<bool> testExpression(&context, "theInt <= largeInt || no");
		doChecks(42 <= 100 || false, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 7")
	{
		Expression<bool> testExpression(&context, "theInt > largeInt || aBoolean");
		doChecks(42 > 100 || true, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 8")
	{
		Expression<bool> testExpression(&context, "theInt >= largeInt || aBoolean");
		doChecks(42 >= 100 || true, false, false, false, testExpression, context);
	}
}

