/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/TypeInfo.h"
#include "PrecompilationTest.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


TEST_CASE("Operator overloading", "[operators][overloading]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("operatorOverloading", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addScope(&reflectedObject, true);	

	SECTION("Overloading operator V4 * V4")
	{
		Expression<TestVector4> testExpression(&context, "getTestVector() * v2");
		doChecks(reflectedObject.getTestVector() * reflectedObject.v2, false, false, false, testExpression, context);
	}
	SECTION("Overloading operator V4 * int")
	{
		Expression<TestVector4> testExpression(&context, "getTestVector() * 42");
		doChecks(reflectedObject.getTestVector() * 42, false, false, false, testExpression, context);
	}
	SECTION("Overloading operator V4 * float")
	{
		Expression<TestVector4> testExpression(&context, "getTestVector() * 11.1f");
		doChecks(reflectedObject.getTestVector() * 11.1f, false, false, false, testExpression, context);
	}
	SECTION("Overloading operator V4 / V4 (static function)")
	{
		Expression<TestVector4> testExpression(&context, "getTestVector() / v2");
		doChecks(reflectedObject.getTestVector() / reflectedObject.v2, false, false, false, testExpression, context);
	}
	SECTION("Overloading operator V4 + V4")
	{
		Expression<TestVector4> testExpression(&context, "v2 + getTestVector()");
		doChecks(reflectedObject.getTestVector() + reflectedObject.v2, false, false, false, testExpression, context);
	}
	SECTION("Overloading operator V4 - V4")
	{
		Expression<TestVector4> testExpression(&context, "v2 - getTestVector()");
		doChecks(reflectedObject.v2 - reflectedObject.getTestVector(), false, false, false, testExpression, context);
	}
	SECTION("Overloading operator []")
	{
		Expression<float> testExpression(&context, "v2[1]");
		doChecks(reflectedObject.v2[1], false, false, false, testExpression, context);
	}
	SECTION("Overloading operator ==")
	{
		Expression<bool> testExpression(&context, "v2 == v1");
		doChecks(reflectedObject.v2 == reflectedObject.v1, false, false, false, testExpression, context);
	}
}

