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


TEST_CASE("Builtin functions test: Random", "[builtins][rand]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Random", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "randStaticScope");	

	SECTION("Random")
	{
		Expression<float> testExpression(&context, "rand()");
		doChecksFn<float>([](float value){return value >= 0.0f && value <= 1.0f;}, false, false, false, testExpression, context);
	}
	SECTION("Random_morearg")
	{
		Expression<float> testExpression(&context, "rand(theInt)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Random_obj")
	{
		Expression<float> testExpression(&context, "rand(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: RandomRange", "[builtins][rand]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_RandomRange", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "randRangStaticScope");	

	SECTION("RandomRange_float_constant")
	{
		Expression<float> testExpression(&context, "rand(101.0f, 102.0f)");
		doChecksFn<float>([&](float value){return value >= 101.0f && value <= 102.0f;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_float_negative_constant")
	{
		Expression<float> testExpression(&context, "rand(-101.0f, -102.0f)");
		doChecksFn<float>([&](float value){return value >= -102.0f && value <= -101.0f;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_float")
	{
		Expression<float> testExpression(&context, "rand(-aFloat, aFloat)");
		doChecksFn<float>([&](float value){return value >= -reflectedObject.aFloat && value <= reflectedObject.aFloat;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_double")
	{
		Expression<double> testExpression(&context, "rand(-aDouble, aDouble)");
		doChecksFn<double>([&](double value){return value >= -reflectedObject.aDouble && value <= reflectedObject.aDouble;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_float_same")
	{
		Expression<float> testExpression(&context, "rand(aFloat, aFloat)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_float_same_negative")
	{
		Expression<float> testExpression(&context, "rand(-aFloat, -aFloat)");
		doChecks(-reflectedObject.aFloat, false, false, false, testExpression, context);
	}

	SECTION("RandomRange_int_constant")
	{
		Expression<int> testExpression(&context, "rand(99, 100)");
		doChecksFn<int>([&](int value){return value >= 99 && value <= 100;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_int_negative_constant")
	{
		Expression<int> testExpression(&context, "rand(-99, -100)");
		doChecksFn<int>([&](int value){return value >= -100 && value <= -99;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_int")
	{
		Expression<int> testExpression(&context, "rand(-theInt, theInt)");
		doChecksFn<int>([&](int value){return value >= -reflectedObject.theInt && value <= reflectedObject.theInt;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_int_same")
	{
		Expression<int> testExpression(&context, "rand(theInt, theInt)");
		doChecks(reflectedObject.theInt, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_int_negative_same")
	{
		Expression<int> testExpression(&context, "rand(-theInt, -theInt)");
		doChecks(-reflectedObject.theInt, false, false, false, testExpression, context);
	}


	SECTION("RandomRange_bool")
	{
		//This test probably always succeeds
		Expression<bool> testExpression(&context, "rand(aBoolean, !aBoolean)");
		doChecksFn<bool>([&](bool value){return value == true || value == false;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_bool")
	{
		//This test probably always succeeds
		Expression<bool> testExpression(&context, "rand(!aBoolean, aBoolean)");
		doChecksFn<bool>([&](bool value){return value == true || value == false;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_bool_same")
	{
		Expression<bool> testExpression(&context, "rand(aBoolean, aBoolean)");
		doChecks(reflectedObject.aBoolean, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_int_negative_same")
	{
		Expression<bool> testExpression(&context, "rand(!aBoolean, !aBoolean)");
		doChecks(!reflectedObject.aBoolean, false, false, false, testExpression, context);
	}

	SECTION("Random_strings")
	{
		Expression<bool> testExpression(&context, "rand(text, numberString)");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Random_morearg")
	{
		Expression<bool> testExpression(&context, "rand(theInt, aFloat, aFloat)");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Random_obj")
	{
		Expression<bool> testExpression(&context, "rand(nestedObject, nestedObject)");
		doChecks(false, true, false, false, testExpression, context);
	}
}
