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


TEST_CASE("Builtin functions test: Abs", "[builtins][abs]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Abs", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "absStaticScope");	

	SECTION("Abs_Constant")
	{
		Expression<float> testExpression(&context, "abs(42.0f)");
		doChecks<float>(std::abs(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Abs_Negative Constant")
	{
		Expression<float> testExpression(&context, "abs(-42.0f)");
		doChecks<float>(std::abs(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Abs_IntConstant")
	{
		Expression<int> testExpression(&context, "abs(3)");
		doChecks(std::abs(3), false, true, false, testExpression, context);
	}
	SECTION("Abs_Negative IntConstant")
	{
		Expression<int> testExpression(&context, "abs(-3)");
		doChecks(std::abs(-3), false, true, false, testExpression, context);
	}
	SECTION("Abs_Variable")
	{
		Expression<int> testExpression(&context, "abs(-33)");
		doChecks(std::abs(-33), false, true, false, testExpression, context);
	}
	SECTION("Abs_Double_Variable")
	{
		Expression<double> testExpression(&context, "abs(aDouble)");
		doChecks(std::abs(reflectedObject.aDouble), false, false, false, testExpression, context);
	}
	SECTION("Abs_Negative_Double_Variable")
	{
		Expression<double> testExpression(&context, "abs(-aDouble)");
		doChecks(std::abs(-reflectedObject.aDouble), false, false, false, testExpression, context);
	}
	SECTION("Abs_Negative Variable")
	{
		Expression<float> testExpression(&context, "abs(-aFloat)");
		doChecks<float>(std::abs(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Abs_Zero Variable")
	{
		Expression<float> testExpression(&context, "abs(zeroFloat)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Abs_IntVariable")
	{
		Expression<int> testExpression(&context, "abs(theInt)");
		doChecks(std::abs(reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Abs_Negative IntVariable")
	{
		Expression<int> testExpression(&context, "abs(-theInt)");
		doChecks(std::abs(-reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Abs_string")
	{
		Expression<int> testExpression(&context, "abs(numberString)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Abs_noarg")
	{
		Expression<int> testExpression(&context, "abs()");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Abs_morearg")
	{
		Expression<int> testExpression(&context, "abs(theInt, aFloat)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Abs_obj")
	{
		Expression<int> testExpression(&context, "abs(nestedObject)");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Sqrt", "[builtins][sqrt]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Sqrt", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "sqrtStaticScope");	

	SECTION("Sqrt_Constant")
	{
		Expression<float> testExpression(&context, "sqrt(42.0f)");
		doChecks<float>(sqrt(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sqrt_Negative Constant")
	{
		Expression<float> testExpression(&context, "sqrt(-42.0f)");
		doChecks<float>(std::numeric_limits<float>::quiet_NaN(), false, true, false, testExpression, context);
	}
	SECTION("Sqrt_IntConstant")
	{
		Expression<float> testExpression(&context, "sqrt(3)");
		doChecks<float>(sqrt(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Sqrt_Negative IntConstant")
	{
		Expression<float> testExpression(&context, "sqrt(-3)");
		doChecks<float>(std::numeric_limits<float>::quiet_NaN(), false, true, false, testExpression, context);
	}
	SECTION("Sqrt_Zero Variable")
	{
		Expression<float> testExpression(&context, "sqrt(zeroFloat)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Sqrt_Variable")
	{
		Expression<float> testExpression(&context, "sqrt(aFloat)");
		doChecks<float>(sqrt(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sqrt_Double_Variable")
	{
		Expression<double> testExpression(&context, "sqrt(aDouble)");
		doChecks(sqrt(reflectedObject.aDouble), false, false, false, testExpression, context);
	}
	SECTION("Sqrt_Negative Variable")
	{
		Expression<float> testExpression(&context, "sqrt(-aFloat)");
		doChecks<float>(std::numeric_limits<float>::quiet_NaN(), false, false, false, testExpression, context);
	}
	SECTION("Sqrt_IntVariable")
	{
		Expression<float> testExpression(&context, "sqrt(theInt)");
		doChecks<float>(sqrt((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Sqrt_Negative IntVariable")
	{
		Expression<float> testExpression(&context, "sqrt(-theInt)");
		doChecks<float>(std::numeric_limits<float>::quiet_NaN(), false, false, false, testExpression, context);
	}
	SECTION("Sqrt_string")
	{
		Expression<float> testExpression(&context, "sqrt(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sqrt_noarg")
	{
		Expression<float> testExpression(&context, "sqrt()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sqrt_morearg")
	{
		Expression<float> testExpression(&context, "sqrt(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sqrt_obj")
	{
		Expression<float> testExpression(&context, "sqrt(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Round", "[builtins][round]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Round", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "roundStaticScope");	

	SECTION("Round_cc1")
	{
		Expression<float> testExpression(&context, "round(11.1f, 1)");
		doChecks(11.1f, false, true, false, testExpression, context);
	}
	SECTION("Round_cc2")
	{
		Expression<float> testExpression(&context, "round(11.1f, 0)");
		doChecks(11.0f, false, true, false, testExpression, context);
	}
	SECTION("Round_cc3")
	{
		Expression<float> testExpression(&context, "round(11.1f, 2)");
		doChecks(11.1f, false, true, false, testExpression, context);
	}
	SECTION("Round_cc4")
	{
		Expression<float> testExpression(&context, "round(-11.1f, 1)");
		doChecks(-11.1f, false, true, false, testExpression, context);
	}
	SECTION("Round_cc5")
	{
		Expression<float> testExpression(&context, "round(-11.1f, 0)");
		doChecks(-11.0f, false, true, false, testExpression, context);
	}
	SECTION("Round_float1")
	{
		Expression<float> testExpression(&context, "round(aFloat, 0)");
		doChecks(CatLinkedIntrinsics::_jc_roundFloat(reflectedObject.aFloat, 0), false, false, false, testExpression, context);
	}
	SECTION("Round_float2")
	{
		Expression<float> testExpression(&context, "round(aFloat, 1.0f)");
		doChecks(CatLinkedIntrinsics::_jc_roundFloat(reflectedObject.aFloat, 1), false, false, false, testExpression, context);
	}
	SECTION("Round_float3")
	{
		Expression<float> testExpression(&context, "round(aFloat, 2)");
		doChecks(CatLinkedIntrinsics::_jc_roundFloat(reflectedObject.aFloat, 2), false, false, false, testExpression, context);
	}
	SECTION("Round_double1")
	{
		Expression<double> testExpression(&context, "round(aDouble, 0)");
		doChecks(CatLinkedIntrinsics::_jc_roundDouble(reflectedObject.aDouble, 0), false, false, false, testExpression, context);
	}
	SECTION("Round_double2")
	{
		Expression<double> testExpression(&context, "round(aDouble, 1.0f)");
		doChecks(CatLinkedIntrinsics::_jc_roundDouble(reflectedObject.aDouble, 1), false, false, false, testExpression, context);
	}
	SECTION("Round_double3")
	{
		Expression<double> testExpression(&context, "round(aDouble, 2)");
		doChecks(CatLinkedIntrinsics::_jc_roundDouble(reflectedObject.aDouble, 2), false, false, false, testExpression, context);
	}
	SECTION("Round_int")
	{
		Expression<float> testExpression(&context, "round(largeInt, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_bool")
	{
		Expression<float> testExpression(&context, "round(aBoolean, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_stringConst")
	{
		Expression<float> testExpression(&context, "round(\"10\", 10)");
		doChecks(0.0f, true, true, false, testExpression, context);
	}
	SECTION("Round_string")
	{
		Expression<float> testExpression(&context, "round(numberString, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_string2")
	{
		Expression<float> testExpression(&context, "round(text, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_noarg")
	{
		Expression<float> testExpression(&context, "round()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_morearg")
	{
		Expression<float> testExpression(&context, "round(theInt, aFloat, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_obj")
	{
		Expression<float> testExpression(&context, "round(nestedObject, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Cap", "[builtins][cap]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Cap", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "CapStaticScope");	

	SECTION("Cap_cc1")
	{
		Expression<float> testExpression(&context, "cap(11.1f, 1, 11)");
		doChecks(11.0f, false, true, false, testExpression, context);
	}
	SECTION("Cap_cc2")
	{
		Expression<int> testExpression(&context, "cap(11, 20, 12)");
		doChecks(12, false, true, false, testExpression, context);
	}
	SECTION("Cap_cc3")
	{
		Expression<float> testExpression(&context, "cap(-11.1f, -11.2, -11)");
		doChecks(-11.1f, false, true, false, testExpression, context);
	}
	SECTION("Cap_cc4")
	{
		Expression<int> testExpression(&context, "cap(-100, -98, -99)");
		doChecks(-99, false, true, false, testExpression, context);
	}
	SECTION("Cap_cc5")
	{
		Expression<int> testExpression(&context, "cap(5, -4.0f, 6.0f)");
		doChecks(5, false, true, false, testExpression, context);
	}
	SECTION("Cap_float1")
	{
		Expression<float> testExpression(&context, "cap(aFloat, 0, 1000)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Cap_float2")
	{
		Expression<float> testExpression(&context, "cap(aFloat, 1.0f, 10)");
		doChecks(10.0f, false, false, false, testExpression, context);
	}
	SECTION("Cap_float3")
	{
		Expression<float> testExpression(&context, "cap(-aFloat, 0.0f, -999.0f)");
		doChecks(-999.0f, false, false, false, testExpression, context);
	}
	SECTION("Cap_double1")
	{
		Expression<double> testExpression(&context, "cap(aDouble, 0, 1000)");
		doChecks(reflectedObject.aDouble, false, false, false, testExpression, context);
	}
	SECTION("Cap_double2")
	{
		Expression<double> testExpression(&context, "cap(aDouble, 1.0, 10)");
		doChecks(10.0, false, false, false, testExpression, context);
	}
	SECTION("Cap_double3")
	{
		Expression<double> testExpression(&context, "cap(-aDouble, 0.0f, -999.0)");
		doChecks(-999.0, false, false, false, testExpression, context);
	}
	SECTION("Cap_int1")
	{
		Expression<int> testExpression(&context, "cap(largeInt, 0, 1)");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("Cap_int2")
	{
		Expression<int> testExpression(&context, "cap(-largeInt, 0, 1)");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Cap_int3")
	{
		Expression<int> testExpression(&context, "cap(largeInt, largeInt, largeInt)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Cap_int4")
	{
		Expression<int> testExpression(&context, "cap(-largeInt, -largeInt, -largeInt)");
		doChecks(-reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Cap_int4")
	{
		Expression<int> testExpression(&context, "cap(largeInt, largeInt + 1, largeInt - 1)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Cap_bool")
	{
		Expression<int> testExpression(&context, "cap(aBoolean, 0, 1)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Cap_stringConst")
	{
		Expression<int> testExpression(&context, "cap(\"10\", 10, 11)");
		doChecks(0, true, true, false, testExpression, context);
	}
	SECTION("Cap_string")
	{
		Expression<float> testExpression(&context, "cap(numberString, 10.0f, 11.0f)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cap_string2")
	{
		Expression<int> testExpression(&context, "cap(text, 10, 12)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Cap_noarg")
	{
		Expression<float> testExpression(&context, "cap()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cap_fewerarg")
	{
		Expression<float> testExpression(&context, "cap(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cap_morearg")
	{
		Expression<float> testExpression(&context, "cap(theInt, aFloat, 10, 11)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cap_obj")
	{
		Expression<float> testExpression(&context, "cap(nestedObject, 10, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Min", "[builtins][min]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Min", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "MinStaticScope");	

	SECTION("Min_cc1")
	{
		Expression<float> testExpression(&context, "min(11.1f, 1)");
		doChecks(1.0f, false, true, false, testExpression, context);
	}
	SECTION("Min_cc2")
	{
		Expression<int> testExpression(&context, "min(0, 11.0f)");
		doChecks(0, false, true, false, testExpression, context);
	}
	SECTION("Min_cc3")
	{
		Expression<int> testExpression(&context, "min(9, -9)");
		doChecks(-9, false, true, false, testExpression, context);
	}
	SECTION("Min_cc4")
	{
		Expression<float> testExpression(&context, "min(-11.1f, 999.0f)");
		doChecks(-11.1f, false, true, false, testExpression, context);
	}
	SECTION("Min_float1")
	{
		Expression<float> testExpression(&context, "min(aFloat, 0)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Min_float2")
	{
		Expression<float> testExpression(&context, "min(aFloat, 1000.0f)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Min_float3")
	{
		Expression<float> testExpression(&context, "min(-aFloat, aFloat)");
		doChecks(-reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Min_double1")
	{
		Expression<double> testExpression(&context, "min(aDouble, 0)");
		doChecks(0.0, false, false, false, testExpression, context);
	}
	SECTION("Min_double2")
	{
		Expression<double> testExpression(&context, "min(aDouble, 1000.0f)");
		doChecks(reflectedObject.aDouble, false, false, false, testExpression, context);
	}
	SECTION("Min_double3")
	{
		Expression<double> testExpression(&context, "min(-aDouble, aFloat)");
		doChecks(-reflectedObject.aDouble, false, false, false, testExpression, context);
	}
	SECTION("Min_int1")
	{
		Expression<int> testExpression(&context, "min(largeInt, 10)");
		doChecks(10, false, false, false, testExpression, context);
	}
	SECTION("Min_int2")
	{
		Expression<int> testExpression(&context, "min(largeInt, -largeInt)");
		doChecks(-reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Min_int3")
	{
		Expression<int> testExpression(&context, "min(largeInt, largeInt)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Min_int4")
	{
		Expression<int> testExpression(&context, "min(largeInt, largeInt + 1)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Min_bool")
	{
		Expression<float> testExpression(&context, "min(aBoolean, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Min_stringConst")
	{
		Expression<float> testExpression(&context, "min(\"10\", 10)");
		doChecks(0.0f, true, true, false, testExpression, context);
	}
	SECTION("Min_string")
	{
		Expression<float> testExpression(&context, "min(numberString, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Min_string2")
	{
		Expression<float> testExpression(&context, "min(text, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Min_noarg")
	{
		Expression<float> testExpression(&context, "min()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Min_morearg")
	{
		Expression<float> testExpression(&context, "min(theInt, aFloat, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Min_obj")
	{
		Expression<float> testExpression(&context, "min(nestedObject, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Max", "[builtins][max]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Max", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "MaxStaticScope");	

	SECTION("Max_cc1")
	{
		Expression<float> testExpression(&context, "max(11.1f, 1)");
		doChecks(11.1f, false, true, false, testExpression, context);
	}
	SECTION("Max_cc2")
	{
		Expression<int> testExpression(&context, "max(0, 11.0f)");
		doChecks(11, false, true, false, testExpression, context);
	}
	SECTION("Max_cc3")
	{
		Expression<int> testExpression(&context, "max(9, -9)");
		doChecks(9, false, true, false, testExpression, context);
	}
	SECTION("Max_cc4")
	{
		Expression<float> testExpression(&context, "max(-11.1f, 999.0f)");
		doChecks(999.0f, false, true, false, testExpression, context);
	}
	SECTION("Max_float1")
	{
		Expression<float> testExpression(&context, "max(aFloat, 0)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Max_float2")
	{
		Expression<float> testExpression(&context, "max(aFloat, 1000.0f)");
		doChecks(1000.0f, false, false, false, testExpression, context);
	}
	SECTION("Max_float3")
	{
		Expression<float> testExpression(&context, "max(-aFloat, aFloat)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Max_double1")
	{
		Expression<double> testExpression(&context, "max(aDouble, 0)");
		doChecks(reflectedObject.aDouble, false, false, false, testExpression, context);
	}
	SECTION("Max_double2")
	{
		Expression<double> testExpression(&context, "max(aDouble, 1000.0f)");
		doChecks(1000.0, false, false, false, testExpression, context);
	}
	SECTION("Max_double3")
	{
		Expression<double> testExpression(&context, "max(-aDouble, aFloat)");
		doChecks(reflectedObject.aDouble, false, false, false, testExpression, context);
	}
	SECTION("Max_int1")
	{
		Expression<int> testExpression(&context, "max(largeInt, 10)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Max_int2")
	{
		Expression<int> testExpression(&context, "max(largeInt, -largeInt)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Max_int3")
	{
		Expression<int> testExpression(&context, "max(largeInt, largeInt)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Max_int4")
	{
		Expression<int> testExpression(&context, "max(largeInt, largeInt + 1)");
		doChecks(reflectedObject.largeInt + 1, false, false, false, testExpression, context);
	}
	SECTION("Max_bool")
	{
		Expression<float> testExpression(&context, "max(aBoolean, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Max_stringConst")
	{
		Expression<float> testExpression(&context, "max(\"10\", 10)");
		doChecks(0.0f, true, true, false, testExpression, context);
	}
	SECTION("Max_string")
	{
		Expression<float> testExpression(&context, "max(numberString, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Max_string2")
	{
		Expression<float> testExpression(&context, "max(text, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Max_noarg")
	{
		Expression<float> testExpression(&context, "max()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Max_morearg")
	{
		Expression<float> testExpression(&context, "max(theInt, aFloat, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Max_obj")
	{
		Expression<float> testExpression(&context, "max(nestedObject, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Log10", "[builtins][log10]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Log10", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "Log10StaticScope");	

	SECTION("Log10_Constant")
	{
		Expression<float> testExpression(&context, "log10(42.0f)");
		doChecks(log10f(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Log10_Negative Constant")
	{
		Expression<float> testExpression(&context, "log10(-42.0f)");
		doChecks<float>(std::numeric_limits<float>::quiet_NaN(), false, true, false, testExpression, context);
	}
	SECTION("Log10_IntConstant")
	{
		Expression<float> testExpression(&context, "log10(3)");
		doChecks(log10f(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Log10_Negative IntConstant")
	{
		Expression<float> testExpression(&context, "log10(-3)");
		doChecks<float>(std::numeric_limits<float>::quiet_NaN(), false, true, false, testExpression, context);
	}
	SECTION("Log10_Zero Variable")
	{
		Expression<float> testExpression(&context, "log10(zeroFloat)");
		doChecks(-std::numeric_limits<float>::infinity(), false, false, false, testExpression, context);
	}
	SECTION("Log10_Variable")
	{
		Expression<float> testExpression(&context, "log10(aFloat)");
		doChecks(log10f(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Log10_Double_Variable")
	{
		Expression<double> testExpression(&context, "log10(aDouble)");
		doChecks(log10(reflectedObject.aDouble), false, false, false, testExpression, context);
	}
	SECTION("Log10_Negative Variable")
	{
		Expression<float> testExpression(&context, "log10(-aFloat)");
		doChecks<float>(std::numeric_limits<float>::quiet_NaN(), false, false, false, testExpression, context);
	}
	SECTION("Log10_IntVariable")
	{
		Expression<float> testExpression(&context, "log10(theInt)");
		doChecks(log10f((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Log10_Negative IntVariable")
	{
		Expression<float> testExpression(&context, "log10(-theInt)");
		doChecks<float>(std::numeric_limits<float>::quiet_NaN(), false, false, false, testExpression, context);
	}
	SECTION("Log10_string")
	{
		Expression<float> testExpression(&context, "log10(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Log10_noarg")
	{
		Expression<float> testExpression(&context, "log10()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Log10_morearg")
	{
		Expression<float> testExpression(&context, "log10(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Log10_obj")
	{
		Expression<float> testExpression(&context, "log10(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Ln", "[builtins][ln]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Ln", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "lnStaticScope");

	SECTION("Ln_Constant")
	{
		Expression<float> testExpression(&context, "ln(42.0f)");
		doChecks(logf(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Ln_Negative_Constant")
	{
		Expression<float> testExpression(&context, "ln(-42.0f)");
		doChecksFn<float>([](float value) {return std::isnan(value); }, false, true, false, testExpression, context);
	}
	SECTION("Ln_Int_Constant")
	{
		Expression<float> testExpression(&context, "ln(3)");
		doChecks(logf(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Ln_Negative_Int_Constant")
	{
		Expression<float> testExpression(&context, "ln(-3)");
		doChecksFn<float>([](float value) {return std::isnan(value); }, false, true, false, testExpression, context);
	}
	SECTION("Ln_Zero_Variable")
	{
		Expression<float> testExpression(&context, "ln(zeroFloat)");
		doChecks(-std::numeric_limits<float>::infinity(), false, false, false, testExpression, context);
	}
	SECTION("Ln_Variable")
	{
		Expression<float> testExpression(&context, "ln(aFloat)");
		doChecks(logf(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Ln_Double_Variable")
	{
		Expression<double> testExpression(&context, "ln(aDouble)");
		doChecks(log(reflectedObject.aDouble), false, false, false, testExpression, context);
	}
	SECTION("Ln_Negative_Variable")
	{
		Expression<float> testExpression(&context, "ln(-aFloat)");
		doChecksFn<float>([](float value) {return std::isnan(value); }, false, false, false, testExpression, context);
	}
	SECTION("Ln_Int_Variable")
	{
		Expression<float> testExpression(&context, "ln(theInt)");
		doChecks(logf((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Ln_Negative_Int_Variable")
	{
		Expression<float> testExpression(&context, "ln(-theInt)");
		doChecksFn<float>([](float value) {return std::isnan(value); }, false, false, false, testExpression, context);
	}
	SECTION("Ln_string")
	{
		Expression<float> testExpression(&context, "ln(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Ln_noarg")
	{
		Expression<float> testExpression(&context, "ln()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Ln_morearg")
	{
		Expression<float> testExpression(&context, "ln(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Ln_obj")
	{
		Expression<float> testExpression(&context, "ln(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Exp", "[builtins][exp]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Exp", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "expStaticScope");

	SECTION("Exp_Constant")
	{
		Expression<float> testExpression(&context, "exp(42.0f)");
		doChecks<float>(exp(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Exp_Negative_Constant")
	{
		Expression<float> testExpression(&context, "exp(-42.0f)");
		doChecks<float>(exp(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Exp_Int_Constant")
	{
		Expression<float> testExpression(&context, "exp(3)");
		doChecks<float>(exp((float)3), false, true, false, testExpression, context);
	}
	SECTION("Exp_Variable")
	{
		Expression<float> testExpression(&context, "exp(aFloat)");
		doChecks<float>(exp(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Exp_Double_Variable")
	{
		Expression<double> testExpression(&context, "exp(aDouble)");
		doChecks(exp(reflectedObject.aDouble), false, false, false, testExpression, context);
	}
	SECTION("Exp_Negative_Variable")
	{
		Expression<float> testExpression(&context, "exp(-aFloat)");
		doChecks<float>(exp(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Exp_Int_Variable")
	{
		Expression<float> testExpression(&context, "exp(theInt)");
		doChecks<float>(exp((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Exp_string")
	{
		Expression<float> testExpression(&context, "exp(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Exp_noarg")
	{
		Expression<float> testExpression(&context, "exp()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Exp_morearg")
	{
		Expression<float> testExpression(&context, "exp(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Exp_obj")
	{
		Expression<float> testExpression(&context, "exp(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Pow", "[builtins][pow]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Pow", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "powStaticScope");	

	SECTION("Pow_cc1")
	{
		Expression<float> testExpression(&context, "pow(11.1f, 1)");
		doChecks<float>((float)pow(11.1f, 1), false, true, false, testExpression, context);
	}
	SECTION("Pow_cc2")
	{
		Expression<float> testExpression(&context, "pow(0, 11.0f)");
		doChecks(0.0f, false, true, false, testExpression, context);
	}
	SECTION("Pow_cc3")
	{
		Expression<float> testExpression(&context, "pow(9, -9)");
		doChecks((float)pow(9, -9), false, true, false, testExpression, context);
	}
	SECTION("Pow_cc4")
	{
		Expression<float> testExpression(&context, "pow(-11.1f, 999.0f)");
		doChecks<float>(pow(-11.1f, 999.0f), false, true, false, testExpression, context);
	}
	SECTION("Pow_float1")
	{
		Expression<float> testExpression(&context, "pow(aFloat, 0)");
		doChecks(1.0f, false, false, false, testExpression, context);
	}
	SECTION("Pow_float2")
	{
		Expression<float> testExpression(&context, "pow(aFloat, 1000.0f)");
		doChecks<float>(pow(reflectedObject.aFloat, 1000.0f), false, false, false, testExpression, context);
	}
	SECTION("Pow_float3")
	{
		Expression<float> testExpression(&context, "pow(-aFloat, aFloat)");
		doChecks<float>(std::numeric_limits<float>::quiet_NaN(), false, false, false, testExpression, context);
	}
	SECTION("Pow_double1")
	{
		Expression<double> testExpression(&context, "pow(aDouble, 0)");
		doChecks(1.0, false, false, false, testExpression, context);
	}
	SECTION("Pow_double2")
	{
		Expression<double> testExpression(&context, "pow(aDouble, 1000.0f)");
		doChecks(pow(reflectedObject.aDouble, 1000.0f), false, false, false, testExpression, context);
	}
	SECTION("Pow_double3")
	{
		Expression<double> testExpression(&context, "pow(-aDouble, aFloat)");
		doChecks(std::numeric_limits<double>::quiet_NaN(), false, false, false, testExpression, context);
	}
	SECTION("Pow_int1")
	{
		Expression<float> testExpression(&context, "pow(largeInt, 10)");
		doChecks((float)pow(reflectedObject.largeInt, 10), false, false, false, testExpression, context);
	}
	SECTION("Pow_int2")
	{
		Expression<float> testExpression(&context, "pow(largeInt, -largeInt)");
		doChecks((float)pow(reflectedObject.largeInt, -reflectedObject.largeInt), false, false, false, testExpression, context);
	}
	SECTION("Pow_int3")
	{
		Expression<float> testExpression(&context, "pow(largeInt, largeInt)");
		doChecks((float)pow(reflectedObject.largeInt, reflectedObject.largeInt), false, false, false, testExpression, context);
	}
	SECTION("Pow_bool")
	{
		Expression<float> testExpression(&context, "pow(aBoolean, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_stringConst")
	{
		Expression<float> testExpression(&context, "pow(\"10\", 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_string")
	{
		Expression<float> testExpression(&context, "pow(numberString, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_string2")
	{
		Expression<float> testExpression(&context, "pow(text, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_noarg")
	{
		Expression<float> testExpression(&context, "pow()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_morearg")
	{
		Expression<float> testExpression(&context, "pow(theInt, aFloat, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_obj")
	{
		Expression<float> testExpression(&context, "pow(nestedObject, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Ceil", "[builtins][ceil]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Ceil", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "ceilStaticScope");	

	SECTION("Ceil_Constant")
	{
		Expression<float> testExpression(&context, "ceil(42.1f)");
		doChecks<float>(ceil(42.1f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_Negative Constant")
	{
		Expression<float> testExpression(&context, "ceil(-42.1f)");
		doChecks<float>(ceil(-42.1f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_IntConstant")
	{
		Expression<float> testExpression(&context, "ceil(3)");
		doChecks<float>(ceil(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_Negative IntConstant")
	{
		Expression<float> testExpression(&context, "ceil(-3)");
		doChecks<float>(ceil(-3.0f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_Zero Variable")
	{
		Expression<float> testExpression(&context, "ceil(zeroFloat)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Ceil_Variable")
	{
		Expression<float> testExpression(&context, "ceil(aFloat)");
		doChecks<float>(ceil(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Ceil_Double_Variable")
	{
		Expression<double> testExpression(&context, "ceil(aDouble)");
		doChecks(ceil(reflectedObject.aDouble), false, false, false, testExpression, context);
	}
	SECTION("Ceil_Negative Variable")
	{
		Expression<float> testExpression(&context, "ceil(-aFloat)");
		doChecks<float>(ceil(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Ceil_IntVariable")
	{
		Expression<float> testExpression(&context, "ceil(theInt)");
		doChecks((float)ceil(reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Ceil_Negative IntVariable")
	{
		Expression<float> testExpression(&context, "ceil(-theInt)");
		doChecks((float)ceil(-reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Ceil_string")
	{
		Expression<float> testExpression(&context, "ceil(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Ceil_noarg")
	{
		Expression<float> testExpression(&context, "ceil()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Ceil_morearg")
	{
		Expression<float> testExpression(&context, "ceil(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Ceil_obj")
	{
		Expression<float> testExpression(&context, "ceil(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Floor", "[builtins][floor]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Floor", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "floorStaticScope");	

	SECTION("Floor_Constant")
	{
		Expression<float> testExpression(&context, "floor(42.1f)");
		doChecks<float>(floor(42.1f), false, true, false, testExpression, context);
	}
	SECTION("Floor_Negative Constant")
	{
		Expression<float> testExpression(&context, "floor(-42.1f)");
		doChecks<float>(floor(-42.1f), false, true, false, testExpression, context);
	}
	SECTION("Floor_IntConstant")
	{
		Expression<float> testExpression(&context, "floor(3)");
		doChecks<float>(floor(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Floor_Negative IntConstant")
	{
		Expression<float> testExpression(&context, "floor(-3)");
		doChecks<float>(floor(-3.0f), false, true, false, testExpression, context);
	}
	SECTION("Floor_Zero Variable")
	{
		Expression<float> testExpression(&context, "floor(zeroFloat)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Floor_Variable")
	{
		Expression<float> testExpression(&context, "floor(aFloat)");
		doChecks<float>(floor(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Floor_Double_Variable")
	{
		Expression<double> testExpression(&context, "floor(aDouble)");
		doChecks(floor(reflectedObject.aDouble), false, false, false, testExpression, context);
	}
	SECTION("Floor_Negative Variable")
	{
		Expression<float> testExpression(&context, "floor(-aFloat)");
		doChecks<float>(floor(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Floor_IntVariable")
	{
		Expression<float> testExpression(&context, "floor(theInt)");
		doChecks((float)floor(reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Floor_Negative IntVariable")
	{
		Expression<float> testExpression(&context, "floor(-theInt)");
		doChecks((float)floor(-reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Floor_string")
	{
		Expression<float> testExpression(&context, "floor(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Floor_noarg")
	{
		Expression<float> testExpression(&context, "floor()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Floor_morearg")
	{
		Expression<float> testExpression(&context, "floor(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Floor_obj")
	{
		Expression<float> testExpression(&context, "floor(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}
