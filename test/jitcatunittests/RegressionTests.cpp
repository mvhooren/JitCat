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



TEST_CASE("Regression testing", "[regression]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	reflectedObject.nestedSelfObject->createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("regressionTests", &errorManager);
	context.addScope(&reflectedObject, true);


	SECTION("Crash when using select with object types.")
	{
		Expression<TestObjects::NestedReflectedObject*> testExpression(&context, "select(!nestedSelfObject.getObject2(text, false).no, nestedSelfObject.getObject2(text, false).nestedObject, nestedObjectPointer)");
		doChecks(&reflectedObject.nestedSelfObject->getObject2(reflectedObject.text, false)->nestedObject, false, false, false, testExpression, context);
	}
	SECTION("Void expression error.")
	{
		Expression<void> testExpression(&context, "rand()");
		doCommonChecks(&testExpression, false, false, false, context);
		testExpression.getValue(&context);
		testExpression.getInterpretedValue(&context);
	}
	SECTION("RandomRange_intAndFloat")
	{
		Expression<float> testExpression(&context, "rand(0, 2.0f)");
		doChecksFn<float>([&](float value) {return value >= 0.0f && value <= 2.0f; }, false, false, false, testExpression, context);
	}
	SECTION("Float zeroLiteral")
	{
		Expression<float> testExpression(&context, "0");
		doChecks(0.0f, false, true, true, testExpression, context);
	}
	SECTION("Int zeroLiteral")
	{
		Expression<int> testExpression(&context, "0.0f");
		doChecks(0, false, true, true, testExpression, context);
	}
	SECTION("Void zeroLiteral")
	{
		Expression<void> testExpression(&context, "0");
		doCommonChecks(&testExpression, false, true, true, context);
	}
	SECTION("Floating point literal with no decimal digits")
	{
		ExpressionAny testExpression(&context, "1.");
		doCommonChecks(&testExpression, false, true, true, context);
		CHECK(testExpression.getType().isFloatType());
	}
	SECTION("Constant with extraneous negative")
	{
		Expression<float> testExpression(&context, "1.-1");
		doChecks(0.0f, false, true, false, testExpression, context);
	}
	SECTION("Constant folding integer 0 multiplied by float variable")
	{
		ExpressionAny testExpression(&context, "0 * aFloat");
		doCommonChecks(&testExpression, false, true, false, context);
		CHECK(testExpression.getType().isFloatType());
	}
	SECTION("Constant folding float zero multiplied by integer variable")
	{
		ExpressionAny testExpression(&context, "0.0f * theInt");
		doCommonChecks(&testExpression, false, true, false, context);
		CHECK(testExpression.getType().isFloatType());
	}
	SECTION("Constant folding float one multiplied by integer variable")
	{
		ExpressionAny testExpression(&context, "1.0f * theInt");
		doCommonChecks(&testExpression, false, false, false, context);
		CHECK(testExpression.getType().isFloatType());
	}
}