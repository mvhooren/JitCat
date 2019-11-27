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


TEST_CASE("Builtin functions test: Sin", "[builtins][sin]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Sin", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Sin_Constant")
	{
		Expression<float> testExpression(&context, "sin(42.0f)");
		doChecks<float>(sin(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sin_Negative Constant")
	{
		Expression<float> testExpression(&context, "sin(-42.0f)");
		doChecks<float>(sin(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sin_IntConstant")
	{
		Expression<float> testExpression(&context, "sin(3)");
		doChecks<float>(sin((float)3), false, true, false, testExpression, context);
	}
	SECTION("Sin_Variable")
	{
		Expression<float> testExpression(&context, "sin(aFloat)");
		doChecks<float>(sin(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sin_Negative Variable")
	{
		Expression<float> testExpression(&context, "sin(-aFloat)");
		doChecks<float>(sin(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sin_IntVariable")
	{
		Expression<float> testExpression(&context, "sin(theInt)");
		doChecks<float>(sin((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Sin_string")
	{
		Expression<float> testExpression(&context, "sin(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sin_noarg")
	{
		Expression<float> testExpression(&context, "sin()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sin_morearg")
	{
		Expression<float> testExpression(&context, "sin(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sin_obj")
	{
		Expression<float> testExpression(&context, "sin(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Cos", "[builtins][cos]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Cos", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Cos_Constant")
	{
		Expression<float> testExpression(&context, "cos(42.0f)");
		doChecks<float>(cos(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Cos_Negative Constant")
	{
		Expression<float> testExpression(&context, "cos(-42.0f)");
		doChecks<float>(cos(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Cos_IntConstant")
	{
		Expression<float> testExpression(&context, "cos(3)");
		doChecks<float>(cos((float)3), false, true, false, testExpression, context);
	}
	SECTION("Cos_Variable")
	{
		Expression<float> testExpression(&context, "cos(aFloat)");
		doChecks<float>(cos(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Cos_Negative Variable")
	{
		Expression<float> testExpression(&context, "cos(-aFloat)");
		doChecks<float>(cos(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Cos_IntVariable")
	{
		Expression<float> testExpression(&context, "cos(theInt)");
		doChecks<float>(cos((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Cos_string")
	{
		Expression<float> testExpression(&context, "cos(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cos_noarg")
	{
		Expression<float> testExpression(&context, "cos()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cos_morearg")
	{
		Expression<float> testExpression(&context, "cos(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cos_obj")
	{
		Expression<float> testExpression(&context, "cos(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Tan", "[builtins][tan]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Tan", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Tan_Constant")
	{
		Expression<float> testExpression(&context, "tan(42.0f)");
		doChecks<float>(tan(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Tan_Negative Constant")
	{
		Expression<float> testExpression(&context, "tan(-42.0f)");
		doChecks<float>(tan(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Tan_IntConstant")
	{
		Expression<float> testExpression(&context, "tan(3)");
		doChecks<float>(tan((float)3), false, true, false, testExpression, context);
	}
	SECTION("Tan_Variable")
	{
		Expression<float> testExpression(&context, "tan(aFloat)");
		doChecks<float>(tan(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Tan_Negative Variable")
	{
		Expression<float> testExpression(&context, "tan(-aFloat)");
		doChecks<float>(tan(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Tan_IntVariable")
	{
		Expression<float> testExpression(&context, "tan(theInt)");
		doChecks<float>(tan((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Tan_string")
	{
		Expression<float> testExpression(&context, "tan(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Tan_noarg")
	{
		Expression<float> testExpression(&context, "tan()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Tan_morearg")
	{
		Expression<float> testExpression(&context, "tan(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Tan_obj")
	{
		Expression<float> testExpression(&context, "abs(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Asin", "[builtins][asin]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Asin", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Asin_Constant")
	{
		Expression<float> testExpression(&context, "asin(0.5f)");
		doChecks<float>(asin(0.5f), false, true, false, testExpression, context);
	}
	SECTION("Asin_Negative_Constant")
	{
		Expression<float> testExpression(&context, "asin(-0.5f)");
		doChecks<float>(asin(-0.5f), false, true, false, testExpression, context);
	}
	SECTION("Asin_Variable")
	{
		Expression<float> testExpression(&context, "asin(smallFloat)");
		doChecks<float>(asin(reflectedObject.smallFloat), false, false, false, testExpression, context);
	}
	SECTION("Asin_Negative_Variable")
	{
		Expression<float> testExpression(&context, "asin(-smallFloat)");
		doChecks<float>(asin(-reflectedObject.smallFloat), false, false, false, testExpression, context);
	}
	SECTION("Asin_string")
	{
		Expression<float> testExpression(&context, "asin(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Asin_noarg")
	{
		Expression<float> testExpression(&context, "asin()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Asin_morearg")
	{
		Expression<float> testExpression(&context, "asin(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Asin_obj")
	{
		Expression<float> testExpression(&context, "asin(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Acos", "[builtins][acos]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Acos", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Acos_Constant")
	{
		Expression<float> testExpression(&context, "acos(0.5f)");
		doChecks<float>(acos(0.5f), false, true, false, testExpression, context);
	}
	SECTION("Acos_Negative_Constant")
	{
		Expression<float> testExpression(&context, "acos(-0.5f)");
		doChecks<float>(acos(-0.5f), false, true, false, testExpression, context);
	}
	SECTION("Acos_Variable")
	{
		Expression<float> testExpression(&context, "acos(smallFloat)");
		doChecks<float>(acos(reflectedObject.smallFloat), false, false, false, testExpression, context);
	}
	SECTION("Acos_Negative_Variable")
	{
		Expression<float> testExpression(&context, "acos(-smallFloat)");
		doChecks<float>(acos(-reflectedObject.smallFloat), false, false, false, testExpression, context);
	}
	SECTION("Acos_string")
	{
		Expression<float> testExpression(&context, "acos(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Acos_noarg")
	{
		Expression<float> testExpression(&context, "acos()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Acos_morearg")
	{
		Expression<float> testExpression(&context, "acos(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Acos_obj")
	{
		Expression<float> testExpression(&context, "acos(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Atan", "[builtins][atan]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Atan", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Atan_Constant")
	{
		Expression<float> testExpression(&context, "atan(42.0f)");
		doChecks<float>(atan(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Atan_Negative_Constant")
	{
		Expression<float> testExpression(&context, "atan(-42.0f)");
		doChecks<float>(atan(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Atan_Int_Constant")
	{
		Expression<float> testExpression(&context, "atan(3)");
		doChecks<float>(atan((float)3), false, true, false, testExpression, context);
	}
	SECTION("Atan_Variable")
	{
		Expression<float> testExpression(&context, "atan(aFloat)");
		doChecks<float>(atan(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Atan_Negative_Variable")
	{
		Expression<float> testExpression(&context, "atan(-aFloat)");
		doChecks<float>(atan(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Atan_Int_Variable")
	{
		Expression<float> testExpression(&context, "atan(theInt)");
		doChecks<float>(atan((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Atan_string")
	{
		Expression<float> testExpression(&context, "atan(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atan_noarg")
	{
		Expression<float> testExpression(&context, "atan()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atan_morearg")
	{
		Expression<float> testExpression(&context, "atan(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atan_obj")
	{
		Expression<float> testExpression(&context, "atan(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Sinh", "[builtins][sinh]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Sinh", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Sinh_Constant")
	{
		Expression<float> testExpression(&context, "sinh(42.0f)");
		doChecks<float>(sinh(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sinh_Negative_Constant")
	{
		Expression<float> testExpression(&context, "sinh(-42.0f)");
		doChecks<float>(sinh(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sinh_Int_Constant")
	{
		Expression<float> testExpression(&context, "sinh(3)");
		doChecks<float>(sinh((float)3), false, true, false, testExpression, context);
	}
	SECTION("Sinh_Variable")
	{
		Expression<float> testExpression(&context, "sinh(aFloat)");
		doChecks<float>(sinh(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sinh_Negative_Variable")
	{
		Expression<float> testExpression(&context, "sinh(-aFloat)");
		doChecks<float>(sinh(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sinh_Int_Variable")
	{
		Expression<float> testExpression(&context, "sinh(theInt)");
		doChecks<float>(sinh((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Sinh_string")
	{
		Expression<float> testExpression(&context, "sinh(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sinh_noarg")
	{
		Expression<float> testExpression(&context, "sinh()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sinh_morearg")
	{
		Expression<float> testExpression(&context, "sinh(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sinh_obj")
	{
		Expression<float> testExpression(&context, "sinh(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Cosh", "[builtins][cosh]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Cosh", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Cosh_Constant")
	{
		Expression<float> testExpression(&context, "cosh(42.0f)");
		doChecks<float>(cosh(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Cosh_Negative_Constant")
	{
		Expression<float> testExpression(&context, "cosh(-42.0f)");
		doChecks<float>(cosh(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Cosh_Int_Constant")
	{
		Expression<float> testExpression(&context, "cosh(3)");
		doChecks<float>(cosh((float)3), false, true, false, testExpression, context);
	}
	SECTION("Cosh_Variable")
	{
		Expression<float> testExpression(&context, "cosh(aFloat)");
		doChecks<float>(cosh(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Cosh_Negative_Variable")
	{
		Expression<float> testExpression(&context, "cosh(-aFloat)");
		doChecks<float>(cosh(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Cosh_Int_Variable")
	{
		Expression<float> testExpression(&context, "cosh(theInt)");
		doChecks<float>(cosh((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Cosh_string")
	{
		Expression<float> testExpression(&context, "cosh(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cosh_noarg")
	{
		Expression<float> testExpression(&context, "cosh()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cosh_morearg")
	{
		Expression<float> testExpression(&context, "cosh(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cosh_obj")
	{
		Expression<float> testExpression(&context, "cosh(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Tanh", "[builtins][tanh]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Tanh", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Tanh_Constant")
	{
		Expression<float> testExpression(&context, "tanh(42.0f)");
		doChecks<float>(tanh(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Tanh_Negative_Constant")
	{
		Expression<float> testExpression(&context, "tanh(-42.0f)");
		doChecks<float>(tanh(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Tanh_Int_Constant")
	{
		Expression<float> testExpression(&context, "tanh(3)");
		doChecks<float>(tanh((float)3), false, true, false, testExpression, context);
	}
	SECTION("Tanh_Variable")
	{
		Expression<float> testExpression(&context, "tanh(aFloat)");
		doChecks<float>(tanh(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Tanh_Negative_Variable")
	{
		Expression<float> testExpression(&context, "tanh(-aFloat)");
		doChecks<float>(tanh(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Tanh_Int_Variable")
	{
		Expression<float> testExpression(&context, "tanh(theInt)");
		doChecks<float>(tanh((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Tanh_string")
	{
		Expression<float> testExpression(&context, "tanh(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Tanh_noarg")
	{
		Expression<float> testExpression(&context, "tanh()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Tanh_morearg")
	{
		Expression<float> testExpression(&context, "tanh(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Tanh_obj")
	{
		Expression<float> testExpression(&context, "tanh(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Asinh", "[builtins][asinh]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Asinh", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Asinh_Constant")
	{
		Expression<float> testExpression(&context, "asinh(0.5f)");
		doChecks<float>(asinh(0.5f), false, true, false, testExpression, context);
	}
	SECTION("Asinh_Negative_Constant")
	{
		Expression<float> testExpression(&context, "asinh(-0.5f)");
		doChecks<float>(asinh(-0.5f), false, true, false, testExpression, context);
	}
	SECTION("Asinh_Variable")
	{
		Expression<float> testExpression(&context, "asinh(smallFloat)");
		doChecks<float>(asinh(reflectedObject.smallFloat), false, false, false, testExpression, context);
	}
	SECTION("Asinh_Variable_2")
	{
		Expression<float> testExpression(&context, "asinh(aFloat)");
		doChecks<float>(asinh(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Asinh_Negative_Variable")
	{
		Expression<float> testExpression(&context, "asinh(-smallFloat)");
		doChecks<float>(asinh(-reflectedObject.smallFloat), false, false, false, testExpression, context);
	}
	SECTION("Asinh_Negative_Variable_2")
	{
		Expression<float> testExpression(&context, "asinh(-aFloat)");
		doChecks<float>(asinh(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Asinh_string")
	{
		Expression<float> testExpression(&context, "asinh(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Asinh_noarg")
	{
		Expression<float> testExpression(&context, "asinh()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Asinh_morearg")
	{
		Expression<float> testExpression(&context, "asinh(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Asinh_obj")
	{
		Expression<float> testExpression(&context, "asinh(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Acosh", "[builtins][acosh]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Acosh", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Acosh_Constant")
	{
		Expression<float> testExpression(&context, "acosh(42.0f)");
		doChecks<float>(acosh(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Acosh_Variable")
	{
		Expression<float> testExpression(&context, "acosh(aFloat)");
		doChecks<float>(acosh(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Acosh_string")
	{
		Expression<float> testExpression(&context, "acosh(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Acosh_noarg")
	{
		Expression<float> testExpression(&context, "acosh()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Acosh_morearg")
	{
		Expression<float> testExpression(&context, "acosh(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Acosh_obj")
	{
		Expression<float> testExpression(&context, "acosh(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Atanh", "[builtins][atanh]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Atanh", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Atanh_Constant")
	{
		Expression<float> testExpression(&context, "atanh(0.5f)");
		doChecks<float>(atanh(0.5f), false, true, false, testExpression, context);
	}
	SECTION("Atanh_Negative_Constant")
	{
		Expression<float> testExpression(&context, "atanh(-0.5f)");
		doChecks<float>(atanh(-0.5f), false, true, false, testExpression, context);
	}
	SECTION("Atanh_Variable")
	{
		Expression<float> testExpression(&context, "atanh(smallFloat)");
		doChecks<float>(atanh(reflectedObject.smallFloat), false, false, false, testExpression, context);
	}
	SECTION("Atanh_Negative_Variable")
	{
		Expression<float> testExpression(&context, "atanh(-smallFloat)");
		doChecks<float>(atanh(-reflectedObject.smallFloat), false, false, false, testExpression, context);
	}
	SECTION("Atanh_string")
	{
		Expression<float> testExpression(&context, "atanh(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atanh_noarg")
	{
		Expression<float> testExpression(&context, "atanh()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atanh_morearg")
	{
		Expression<float> testExpression(&context, "atanh(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atanh_obj")
	{
		Expression<float> testExpression(&context, "atanh(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Atan2", "[builtins][atan2]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Atan2", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Atan2_cc1")
	{
		Expression<float> testExpression(&context, "atan2(11.1f, 1)");
		doChecks<float>(atan2(11.1f, 1.f), false, true, false, testExpression, context);
	}
	SECTION("Atan2_cc2")
	{
		Expression<float> testExpression(&context, "atan2(0, 11.0f)");
		doChecks<float>(atan2(0.f, 11.0f), false, true, false, testExpression, context);
	}
	SECTION("Atan2_cc3")
	{
		Expression<float> testExpression(&context, "atan2(9, -9)");
		doChecks<float>(atan2(9.f, -9.f), false, true, false, testExpression, context);
	}
	SECTION("Atan2_cc4")
	{
		Expression<float> testExpression(&context, "atan2(-11.1f, 999.0f)");
		doChecks<float>(atan2(-11.1f, 999.0f), false, true, false, testExpression, context);
	}
	SECTION("Atan2_float_int")
	{
		Expression<float> testExpression(&context, "atan2(aFloat, theInt)");
		doChecks<float>(atan2(reflectedObject.aFloat, (float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Atan2_int_float")
	{
		Expression<float> testExpression(&context, "atan2(theInt, aFloat)");
		doChecks<float>(atan2((float)reflectedObject.theInt, reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Atan2_float_const")
	{
		Expression<float> testExpression(&context, "atan2(aFloat, 1000.0f)");
		doChecks<float>(atan2(reflectedObject.aFloat, 1000.0f), false, false, false, testExpression, context);
	}
	SECTION("Atan2_float_float")
	{
		Expression<float> testExpression(&context, "atan2(-aFloat, aFloat)");
		doChecks<float>(atan2(-reflectedObject.aFloat, reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Atan2_int_const")
	{
		Expression<float> testExpression(&context, "atan2(theInt, 10)");
		doChecks<float>(atan2((float)reflectedObject.theInt, (float)10), false, false, false, testExpression, context);
	}
	SECTION("Atan2_int_int")
	{
		Expression<float> testExpression(&context, "atan2(theInt, largeInt)");
		doChecks<float>(atan2((float)reflectedObject.theInt, (float)reflectedObject.largeInt), false, false, false, testExpression, context);
	}
	SECTION("Atan2_int_int2")
	{
		Expression<float> testExpression(&context, "atan2(-theInt, -largeInt)");
		doChecks<float>(atan2((float)-reflectedObject.theInt, (float)-reflectedObject.largeInt), false, false, false, testExpression, context);
	}
	SECTION("Atan2_bool")
	{
		Expression<float> testExpression(&context, "atan2(aBoolean, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atan2_stringConst")
	{
		Expression<float> testExpression(&context, "atan2(\"10\", 10)");
		doChecks(0.0f, true, true, false, testExpression, context);
	}
	SECTION("Atan2_string")
	{
		Expression<float> testExpression(&context, "atan2(numberString, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atan2_string2")
	{
		Expression<float> testExpression(&context, "atan2(text, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atan2_noarg")
	{
		Expression<float> testExpression(&context, "atan2()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atan2_morearg")
	{
		Expression<float> testExpression(&context, "atan2(theInt, aFloat, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Atan2_obj")
	{
		Expression<float> testExpression(&context, "atan2(nestedObject, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Hypot", "[builtins][hypot]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Hypot", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Hypot_cc1")
	{
		Expression<float> testExpression(&context, "hypot(11.1f, 1)");
		doChecks<float>(hypot(11.1f, 1.f), false, true, false, testExpression, context);
	}
	SECTION("Hypot_cc2")
	{
		Expression<float> testExpression(&context, "hypot(0, 11.0f)");
		doChecks<float>(hypot(0.f, 11.0f), false, true, false, testExpression, context);
	}
	SECTION("Hypot_cc3")
	{
		Expression<float> testExpression(&context, "hypot(9, -9)");
		doChecks<float>(hypot(9.f, -9.f), false, true, false, testExpression, context);
	}
	SECTION("Hypot_cc4")
	{
		Expression<float> testExpression(&context, "hypot(-11.1f, 999.0f)");
		doChecks<float>(hypot(-11.1f, 999.0f), false, true, false, testExpression, context);
	}
	SECTION("Hypot_float_int")
	{
		Expression<float> testExpression(&context, "hypot(aFloat, theInt)");
		doChecks<float>(hypot(reflectedObject.aFloat, (float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Hypot_int_float")
	{
		Expression<float> testExpression(&context, "hypot(theInt, aFloat)");
		doChecks<float>(hypot((float)reflectedObject.theInt, reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Hypot_float_const")
	{
		Expression<float> testExpression(&context, "hypot(aFloat, 1000.0f)");
		doChecks<float>(hypot(reflectedObject.aFloat, 1000.0f), false, false, false, testExpression, context);
	}
	SECTION("Hypot_float_float")
	{
		Expression<float> testExpression(&context, "hypot(-aFloat, aFloat)");
		doChecks<float>(hypot(-reflectedObject.aFloat, reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Hypot_int_const")
	{
		Expression<float> testExpression(&context, "hypot(theInt, 10)");
		doChecks<float>(hypot((float)reflectedObject.theInt, (float)10), false, false, false, testExpression, context);
	}
	SECTION("Hypot_int_int")
	{
		Expression<float> testExpression(&context, "hypot(theInt, largeInt)");
		doChecks<float>(hypot((float)reflectedObject.theInt, (float)reflectedObject.largeInt), false, false, false, testExpression, context);
	}
	SECTION("Hypot_int_int2")
	{
		Expression<float> testExpression(&context, "hypot(-theInt, -largeInt)");
		doChecks<float>(hypot((float)-reflectedObject.theInt, (float)-reflectedObject.largeInt), false, false, false, testExpression, context);
	}
	SECTION("Hypot_bool")
	{
		Expression<float> testExpression(&context, "hypot(aBoolean, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Hypot_stringConst")
	{
		Expression<float> testExpression(&context, "hypot(\"10\", 10)");
		doChecks(0.0f, true, true, false, testExpression, context);
	}
	SECTION("Hypot_string")
	{
		Expression<float> testExpression(&context, "hypot(numberString, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Hypot_string2")
	{
		Expression<float> testExpression(&context, "hypot(text, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Hypot_noarg")
	{
		Expression<float> testExpression(&context, "hypot()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Hypot_morearg")
	{
		Expression<float> testExpression(&context, "hypot(theInt, aFloat, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Hypot_obj")
	{
		Expression<float> testExpression(&context, "hypot(nestedObject, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}