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


TEST_CASE("Builtin functions test: ToInt", "[builtins][toint]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToInt", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToInt_cc")
	{
		Expression<int> testExpression(&context, "toInt(11.1f)");
		doChecks(11, false, true, false, testExpression, context);
	}
	SECTION("ToInt_float")
	{
		Expression<int> testExpression(&context, "toInt(aFloat)");
		doChecks((int)reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("ToInt_int")
	{
		Expression<int> testExpression(&context, "toInt(theInt)");
		doChecks(reflectedObject.theInt, false, false, false, testExpression, context);
	}
	SECTION("ToInt_bool")
	{
		Expression<int> testExpression(&context, "toInt(aBoolean)");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("ToInt_stringConst")
	{
		Expression<int> testExpression(&context, "toInt(\"10\")");
		doChecks(10, false, true, false, testExpression, context);
	}
	SECTION("ToInt_string")
	{
		Expression<int> testExpression(&context, "toInt(numberString)");
		doChecks(123, false, false, false, testExpression, context);
	}
	SECTION("ToInt_string2")
	{
		Expression<int> testExpression(&context, "toInt(text)");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("ToInt_noarg")
	{
		Expression<int> testExpression(&context, "toInt()");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("ToInt_morearg")
	{
		Expression<int> testExpression(&context, "toInt(theInt, aFloat)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("ToInt_obj")
	{
		Expression<int> testExpression(&context, "toInt(nestedObject)");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToFloat", "[builtins][tofloat]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToFloat", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToFloat_cc")
	{
		Expression<float> testExpression(&context, "toFloat(11.1f)");
		doChecks(11.1f, false, true, false, testExpression, context);
	}
	SECTION("ToFloat_float")
	{
		Expression<float> testExpression(&context, "toFloat(aFloat)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("ToFloat_int")
	{
		Expression<float> testExpression(&context, "toFloat(theInt)");
		doChecks((float)reflectedObject.theInt, false, false, false, testExpression, context);
	}
	SECTION("ToFloat_bool")
	{
		Expression<float> testExpression(&context, "toFloat(aBoolean)");
		doChecks(1.0f, false, false, false, testExpression, context);
	}
	SECTION("ToFloat_stringConst")
	{
		Expression<float> testExpression(&context, "toFloat(\"10\")");
		doChecks(10.0f, false, true, false, testExpression, context);
	}
	SECTION("ToFloat_string")
	{
		Expression<float> testExpression(&context, "toFloat(numberString)");
		doChecks(123.4f, false, false, false, testExpression, context);
	}
	SECTION("ToFloat_string2")
	{
		Expression<float> testExpression(&context, "toFloat(text)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("ToFloat_noarg")
	{
		Expression<float> testExpression(&context, "toFloat()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("ToFloat_morearg")
	{
		Expression<float> testExpression(&context, "toFloat(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("ToFloat_obj")
	{
		Expression<float> testExpression(&context, "toFloat(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToBool", "[builtins][tobool]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToBool", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToBool_cc")
	{
		Expression<bool> testExpression(&context, "toBool(11.1f)");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("ToBool_cc")
	{
		Expression<bool> testExpression(&context, "toBool(-11.1f)");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("ToBool_cc")
	{
		Expression<bool> testExpression(&context, "toBool(0.0f)");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("ToBool_float")
	{
		Expression<bool> testExpression(&context, "toBool(aFloat)");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("ToBool_int")
	{
		Expression<bool> testExpression(&context, "toBool(theInt)");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("ToBool_int")
	{
		Expression<bool> testExpression(&context, "toBool(-theInt)");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("ToBool_int")
	{
		Expression<bool> testExpression(&context, "toBool(0)");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("ToBool_bool")
	{
		Expression<bool> testExpression(&context, "toBool(aBoolean)");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("ToBool_bool")
	{
		Expression<bool> testExpression(&context, "toBool(false)");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("ToBool_stringConst")
	{
		Expression<bool> testExpression(&context, "toBool(\"10\")");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("ToBool_stringConst")
	{
		Expression<bool> testExpression(&context, "toBool(\"\")");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("ToBool_string")
	{
		Expression<bool> testExpression(&context, "toBool(numberString)");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("ToBool_string2")
	{
		Expression<bool> testExpression(&context, "toBool(text)");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("ToBool_noarg")
	{
		Expression<bool> testExpression(&context, "toBool()");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("ToBool_morearg")
	{
		Expression<bool> testExpression(&context, "toBool(theInt, aFloat)");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("ToBool_obj")
	{
		Expression<bool> testExpression(&context, "toBool(nestedObject)");
		doChecks(false, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToString", "[builtins][tostring]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToString_cc")
	{
		Expression<std::string> testExpression(&context, "toString(11.1f)");
		doChecks(Tools::makeString(11.1f), false, true, false, testExpression, context);
	}
	SECTION("ToString_float")
	{
		Expression<std::string> testExpression(&context, "toString(aFloat)");
		doChecks(Tools::makeString(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("ToString_int")
	{
		Expression<std::string> testExpression(&context, "toString(theInt)");
		doChecks(Tools::makeString(reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("ToString_bool")
	{
		Expression<std::string> testExpression(&context, "toString(aBoolean)");
		doChecks(std::string("1"), false, false, false, testExpression, context);
	}
	SECTION("ToString_stringConst")
	{
		Expression<std::string> testExpression(&context, "toString(\"10\")");
		doChecks(std::string("10"), false, true, false, testExpression, context);
	}
	SECTION("ToString_string")
	{
		Expression<std::string> testExpression(&context, "toString(numberString)");
		doChecks(reflectedObject.numberString, false, false, false, testExpression, context);
	}
	SECTION("ToString_string2")
	{
		Expression<std::string> testExpression(&context, "toString(text)");
		doChecks(reflectedObject.text, false, false, false, testExpression, context);
	}
	SECTION("ToString_noarg")
	{
		Expression<std::string> testExpression(&context, "toString()");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToString_morearg")
	{
		Expression<std::string> testExpression(&context, "toString(theInt, aFloat)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToString_obj")
	{
		Expression<std::string> testExpression(&context, "toString(nestedObject)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToPrettyString", "[builtins][toprettystring]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToPrettyString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToPrettyString_cc")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(11.1f)");
		doChecks(Tools::makeString(11.1f), false, true, false, testExpression, context);
	}
	SECTION("ToPrettyString_float")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(aFloat)");
		doChecks(Tools::makeString(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_int")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(largeInt)");
		doChecks(LLVMCatIntrinsics::intToPrettyString(reflectedObject.largeInt), false, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_bool")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(aBoolean)");
		doChecks(std::string("1"), false, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_stringConst")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(\"10\")");
		doChecks(std::string("10"), false, true, false, testExpression, context);
	}
	SECTION("ToPrettyString_string")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(numberString)");
		doChecks(reflectedObject.numberString, false, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_string2")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(text)");
		doChecks(reflectedObject.text, false, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_noarg")
	{
		Expression<std::string> testExpression(&context, "toPrettyString()");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_morearg")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(theInt, aFloat)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_obj")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(nestedObject)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToFixedLengthString", "[builtins][tofixedlengthstring]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToFixedLengthString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToFixedLengthString_cc")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(11.1f, 10)");
		doChecks(Tools::makeString(11.1f), true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_float")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(aFloat, 10)");
		doChecks(Tools::makeString(reflectedObject.aFloat), true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_int")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(largeInt, 10)");
		doChecks(LLVMCatIntrinsics::intToFixedLengthString(reflectedObject.largeInt, 10), false, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_bool")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(aBoolean, 10)");
		doChecks(std::string("1"), true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_stringConst")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(\"10\", 10)");
		doChecks(std::string("10"), true, true, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_string")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(numberString, 10)");
		doChecks(reflectedObject.numberString, true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_string2")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(text, 10)");
		doChecks(reflectedObject.text, true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_noarg")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString()");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_morearg")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(theInt, aFloat, 10)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_obj")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(nestedObject, 10)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
}
