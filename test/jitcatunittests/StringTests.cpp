/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Configuration.h"
#include "jitcat/TypeInfo.h"
#include "PrecompilationTest.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


TEST_CASE("String Tests", "[string][operators]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("stringTests", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "stringStaticScope");

	SECTION("Constant")
	{
		Expression<std::string> testExpression(&context, "\"hello\"");
		doChecks(std::string("hello"), false, true, true, testExpression, context);
	}
	SECTION("Constant addition int")
	{
		Expression<std::string> testExpression(&context, "\"hello\" + 2");
		doChecks(std::string("hello2"), false, false, false , testExpression, context);
	}
	SECTION("Constant addition float")
	{
		Expression<std::string> testExpression(&context, "\"hello\" + 2.1f");
		doChecks(std::string("hello2.1"), false, false, false, testExpression, context);
	}
	SECTION("Constant addition double")
	{
		Expression<std::string> testExpression(&context, "\"hello\" + ToDouble(2.1)");
		doChecks(std::string("hello2.1"), false, false, false, testExpression, context);
	}
	SECTION("Constant addition bool")
	{
		Expression<std::string> testExpression(&context, "\"hello\" + false");
		doChecks(std::string("hello0"), false, false, false, testExpression, context);
	}
	SECTION("Constant addition string")
	{
		Expression<std::string> testExpression(&context, "\"hello\" + \"test\"");
		doChecks(std::string("hellotest"), false, false, false, testExpression, context);
	}

	SECTION("Constant reverse addition int")
	{
		Expression<std::string> testExpression(&context, "2 + \"hello\"");
		doChecks(std::string("2hello"), false, false, false, testExpression, context);
	}
	SECTION("Constant reverse addition float")
	{
		Expression<std::string> testExpression(&context, "2.1f + \"hello\"");
		doChecks(std::string("2.1hello"), false, false, false, testExpression, context);
	}
	SECTION("Constant reverse addition bool")
	{
		Expression<std::string> testExpression(&context, "true + \"hello\"");
		doChecks(std::string("1hello"), false, false, false, testExpression, context);
	}

	SECTION("Static Constant")
	{
		Expression<std::string> testExpression(&context, "stringConstant");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("Variable")
	{
		Expression<std::string> testExpression(&context, "text");
		doChecks(std::string("Hello!"), false, false, false, testExpression, context);
	}
	SECTION("Variable addition int constant")
	{
		Expression<std::string> testExpression(&context, "text + 2");
		doChecks(std::string("Hello!2"), false, false, false, testExpression, context);
	}
	SECTION("Variable addition float constant")
	{
		Expression<std::string> testExpression(&context, "text + 2.1f");
		doChecks(std::string("Hello!2.1"), false, false, false, testExpression, context);
	}
	SECTION("Variable addition bool constant")
	{
		Expression<std::string> testExpression(&context, "text + false");
		doChecks(std::string("Hello!0"), false, false, false, testExpression, context);
	}
	SECTION("Variable addition string constant")
	{
		Expression<std::string> testExpression(&context, "text + \"test\"");
		doChecks(std::string("Hello!test"), false, false, false, testExpression, context);
	}

	SECTION("Variable addition variable")
	{
		Expression<std::string> testExpression(&context, "text");
		doChecks(std::string("Hello!"), false, false, false, testExpression, context);
	}
	SECTION("Variable addition int variable")
	{
		Expression<std::string> testExpression(&context, "text + theInt");
		doChecks(std::string("Hello!42"), false, false, false, testExpression, context);
	}
	SECTION("Variable addition float variable")
	{
		Expression<std::string> testExpression(&context, "text + aFloat");
		doChecks(std::string("Hello!999.9"), false, false, false, testExpression, context);
	}
	SECTION("Variable addition bool variable")
	{
		Expression<std::string> testExpression(&context, "text + no");
		doChecks(std::string("Hello!0"), false, false, false, testExpression, context);
	}
	SECTION("Variable addition string variable")
	{
		Expression<std::string> testExpression(&context, "text + numberstring");
		doChecks(std::string("Hello!123.4"), false, false, false, testExpression, context);
	}

	SECTION("Constant comparison false")
	{
		Expression<bool> testExpression(&context, "\"hello\" == \"world\"");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Constant comparison true")
	{
		Expression<bool> testExpression(&context, "\"hello\" == \"hello\"");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable comparison true")
	{
		Expression<bool> testExpression(&context, "\"Hello!\" == text");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable comparison true 2")
	{
		Expression<bool> testExpression(&context, "text == \"Hello!\"");
		doChecks(true, false, false, false, testExpression, context);
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable comparison false")
	{
		Expression<bool> testExpression(&context, "\"World!\" == text");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable comparison false 2")
	{
		Expression<bool> testExpression(&context, "text == \"World!\"");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable-variable comparison false")
	{
		Expression<bool> testExpression(&context, "numberString == text");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable-variable comparison true")
	{
		Expression<bool> testExpression(&context, "text == text");
		doChecks(true, false, false, false, testExpression, context);
	}


	SECTION("Constant not-comparison true")
	{
		Expression<bool> testExpression(&context, "\"hello\" != \"world\"");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Constant not-comparison false")
	{
		Expression<bool> testExpression(&context, "\"hello\" != \"hello\"");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable not-comparison false")
	{
		Expression<bool> testExpression(&context, "\"Hello!\" != text");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable not-comparison false 2")
	{
		Expression<bool> testExpression(&context, "text != \"Hello!\"");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable not-comparison true")
	{
		Expression<bool> testExpression(&context, "\"World!\" != text");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable not-comparison true 2")
	{
		Expression<bool> testExpression(&context, "text != \"World!\"");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable-variable not-comparison true")
	{
		Expression<bool> testExpression(&context, "numberString != text");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable-variable not-comparison false")
	{
		Expression<bool> testExpression(&context, "text != text");
		doChecks(false, false, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: StringRound", "[builtins][stringround]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_StringRound", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "StringRoundStaticScope");	

	SECTION("StringRound_cc1")
	{
		Expression<std::string> testExpression(&context, "stringRound(11.1f, 1)");
		doChecks(std::string("11.1"), false, true, false, testExpression, context);
	}
	SECTION("StringRound_cc2")
	{
		Expression<std::string> testExpression(&context, "stringRound(11.1f, 0)");
		doChecks(std::string("11"), false, true, false, testExpression, context);
	}
	SECTION("StringRound_cc3")
	{
		Expression<std::string> testExpression(&context, "stringRound(11.1f, 2)");
		doChecks(std::string("11.1"), false, true, false, testExpression, context);
	}
	SECTION("Round_cc4")
	{
		Expression<std::string> testExpression(&context, "stringRound(-11.1f, 1)");
		doChecks(std::string("-11.1"), false, true, false, testExpression, context);
	}
	SECTION("Round_cc5")
	{
		Expression<std::string> testExpression(&context, "stringRound(-11.1f, 0)");
		doChecks(std::string("-11"), false, true, false, testExpression, context);
	}
	SECTION("StringRound_float1")
	{
		Expression<std::string> testExpression(&context, "stringRound(aFloat, 0)");
		doChecks(LLVMCatIntrinsics::roundFloatToString(reflectedObject.aFloat, 0), false, false, false, testExpression, context);
	}
	SECTION("StringRound_float2")
	{
		Expression<std::string> testExpression(&context, "stringRound(aFloat, 1.0f)");
		doChecks(LLVMCatIntrinsics::roundFloatToString(reflectedObject.aFloat, 1), false, false, false, testExpression, context);
	}
	SECTION("StringRound_float3")
	{
		Expression<std::string> testExpression(&context, "stringRound(aFloat, 2)");
		doChecks(LLVMCatIntrinsics::roundFloatToString(reflectedObject.aFloat, 1), false, false, false, testExpression, context);
	}
	SECTION("StringRound_double1")
	{
		Expression<std::string> testExpression(&context, "stringRound(aDouble, 0)");
		doChecks(LLVMCatIntrinsics::roundDoubleToString(reflectedObject.aDouble, 0), false, false, false, testExpression, context);
	}
	SECTION("StringRound_double2")
	{
		Expression<std::string> testExpression(&context, "stringRound(aDouble, 1.0f)");
		doChecks(LLVMCatIntrinsics::roundDoubleToString(reflectedObject.aDouble, 1), false, false, false, testExpression, context);
	}
	SECTION("StringRound_double3")
	{
		Expression<std::string> testExpression(&context, "stringRound(aDouble, 2)");
		doChecks(LLVMCatIntrinsics::roundDoubleToString(reflectedObject.aDouble, 1), false, false, false, testExpression, context);
	}
	SECTION("StringRound_int")
	{
		Expression<std::string> testExpression(&context, "stringRound(largeInt, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_bool")
	{
		Expression<std::string> testExpression(&context, "stringRound(aBoolean, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_stringConst")
	{
		Expression<std::string> testExpression(&context, "stringRound(\"10\", 10)");
		doChecks(std::string(""), true, true, false, testExpression, context);
	}
	SECTION("StringRound_string")
	{
		Expression<std::string> testExpression(&context, "stringRound(numberString, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_string2")
	{
		Expression<std::string> testExpression(&context, "stringRound(text, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_noarg")
	{
		Expression<std::string> testExpression(&context, "stringRound()");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_morearg")
	{
		Expression<std::string> testExpression(&context, "stringRound(theInt, aFloat, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_obj")
	{
		Expression<std::string> testExpression(&context, "stringRound(nestedObject, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: find", "[builtins][string.find]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_String.find", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "stringFindStaticScope");	

	SECTION("String.find_cc1")
	{
		Expression<int> testExpression(&context, "ToString(11.1f).find(ToString(1))");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("String.find_cc2")
	{
		Expression<int> testExpression(&context, "ToString(0).find(ToString(11.0f))");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("String.find_cc3")
	{
		Expression<int> testExpression(&context, "ToString(9).find(ToString(-9))");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("String.find_cc4")
	{
		Expression<int> testExpression(&context, "\"test\".find(\"t\")");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("String.find_cc5")
	{
		Expression<int> testExpression(&context, "\"test\".find(\"est\")");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("String.find_cc6")
	{
		Expression<int> testExpression(&context, "\"test\".find(\"xxx\")");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("String.find_float1")
	{
		Expression<int> testExpression(&context, "ToString(aFloat).find(ToString(9))");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("String.find_float2")
	{
		Expression<int> testExpression(&context, "ToString(aFloat).find(ToString(1000.0f))");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("String.find_int1")
	{
		Expression<int> testExpression(&context, "ToString(largeInt).find(ToString(7))");
		doChecks(6, false, false, false, testExpression, context);
	}
	SECTION("String.find_int2")
	{
		Expression<int> testExpression(&context, "ToString(largeInt).find(ToString(-largeInt))");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("String.find_int3")
	{
		Expression<int> testExpression(&context, "ToString(largeInt).find(ToString(largeInt))");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("String.find_bool")
	{
		Expression<int> testExpression(&context, "ToString(aBoolean).find(\"1\")");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("String.find_stringConst")
	{
		Expression<int> testExpression(&context, "\"10\".find(ToString(10))");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("String.find_string")
	{
		Expression<int> testExpression(&context, "numberString.find(\".4\")");
		doChecks(3, false, false, false, testExpression, context);
	}
	SECTION("String.find_string2")
	{
		Expression<int> testExpression(&context, "text.find(\"ll\")");
		doChecks(2, false, false, false, testExpression, context);
	}
	SECTION("String.find_nullString")
	{
		Expression<int> testExpression(&context, "nullObject.text.find(\"ll\")");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("String.find_nullArg")
	{
		Expression<int> testExpression(&context, "text.find(nullObject.text)");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("String.find_noarg")
	{
		Expression<int> testExpression(&context, "text.find()");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("String.find_morearg")
	{
		Expression<int> testExpression(&context, "text.find(theInt, aFloat, 10)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("String.find_obj")
	{
		Expression<int> testExpression(&context, "text.find(nestedObject, 10)");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: String.replace", "[builtins][String.replace]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_String.replace", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "StringReplaceStaticScope");	

	SECTION("String.replace_cc1")
	{
		Expression<std::string> testExpression(&context, "ToString(11.1f).replace(\"1\", \"2\")");
		doChecks(std::string("22.2"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_cc2")
	{
		Expression<std::string> testExpression(&context, "ToString(0).replace(ToString(11.0f), ToString(12))");
		doChecks(std::string("0"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_cc3")
	{
		Expression<std::string> testExpression(&context, "\"9\".replace(ToString(9), \"7\")");
		doChecks(std::string("7"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_cc4")
	{
		Expression<std::string> testExpression(&context, "\"test\".replace(\"t\", \"tt\")");
		doChecks(std::string("ttestt"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_cc5")
	{
		Expression<std::string> testExpression(&context, "\"test\".replace(\"est\", \"om\")");
		doChecks(std::string("tom"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_cc6")
	{
		Expression<std::string> testExpression(&context, "\"test\".replace(\"xxx\", ToString(false))");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_float1")
	{
		Expression<std::string> testExpression(&context, "ToString(aFloat).replace(ToString(9), toString(true))");
		doChecks(std::string("111.1"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_float2")
	{
		Expression<std::string> testExpression(&context, "ToString(aFloat).replace(ToString(1000.0f), \"test\")");
		doChecks(std::string("999.9"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_int1")
	{
		Expression<std::string> testExpression(&context, "ToString(largeInt).replace(\"7\", \"789\")");
		doChecks(std::string("123456789"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_int2")
	{
		Expression<std::string> testExpression(&context, "ToString(largeInt).replace(ToString(-largeInt), \"0\")");
		doChecks(std::string("1234567"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_int3")
	{
		Expression<std::string> testExpression(&context, "ToString(largeInt).replace(ToString(largeInt), ToString(largeInt))");
		doChecks(std::string("1234567"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_bool")
	{
		Expression<std::string> testExpression(&context, "ToString(aBoolean).replace(\"1\", ToString(false))");
		doChecks(std::string("0"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_stringConst")
	{
		Expression<std::string> testExpression(&context, "\"10\".replace(\"10\", \"42\")");
		doChecks(std::string("42"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_string")
	{
		Expression<std::string> testExpression(&context, "numberString.replace(\".4\", \"4\")");
		doChecks(std::string("1234"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_string2")
	{
		Expression<std::string> testExpression(&context, "text.replace(\"ll\", text)");
		doChecks(std::string("HeHello!o!"), false, false, false, testExpression, context);
	}
	SECTION("String.replace_nullString")
	{
		Expression<std::string> testExpression(&context, "nullObject.text.replace(\"ll\", text)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("String.replace_nullArgs")
	{
		Expression<std::string> testExpression(&context, "text.replace(nullObject.text, nullObject.text)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("String.replace_noarg")
	{
		Expression<std::string> testExpression(&context, "text.replace()");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("String.replace_morearg")
	{
		Expression<std::string> testExpression(&context, "text.replace(theInt, aFloat, 10, false)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("String.replace_obj")
	{
		Expression<std::string> testExpression(&context, "text.replace(nestedObject, 10, false)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: String.length", "[builtins][String.length]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_String.length", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "StringLengthStaticScope");	

	SECTION("String.length_cc1")
	{
		Expression<int> testExpression(&context, "ToString(11.1f).length()");
		doChecks(4, false, false, false, testExpression, context);
	}
	SECTION("String.length_cc2")
	{
		Expression<int> testExpression(&context, "ToString(0).length()");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("String.length_cc3")
	{
		Expression<int> testExpression(&context, "ToString(9).length()");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("String.length_cc4")
	{
		Expression<int> testExpression(&context, "\"test\".length()");
		doChecks(4, false, false, false, testExpression, context);
	}
	SECTION("String.length_cc5")
	{
		Expression<int> testExpression(&context, "\"\".length()");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("String.length_float1")
	{
		Expression<int> testExpression(&context, "ToString(aFloat).length()");
		doChecks(5, false, false, false, testExpression, context);
	}
	SECTION("String.length_float2")
	{
		Expression<int> testExpression(&context, "ToString(zeroFloat).length()");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("String.length_int1")
	{
		Expression<int> testExpression(&context, "ToString(largeInt).length()");
		doChecks(7, false, false, false, testExpression, context);
	}
	SECTION("String.length_int2")
	{
		Expression<int> testExpression(&context, "ToString(-largeInt).length()");
		doChecks(8, false, false, false, testExpression, context);
	}
	SECTION("String.length_bool")
	{
		Expression<int> testExpression(&context, "ToString(aBoolean).length()");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("String.length_stringConst")
	{
		Expression<int> testExpression(&context, "\"10\".length()");
		doChecks(2, false, false, false, testExpression, context);
	}
	SECTION("String.length_string")
	{
		Expression<int> testExpression(&context, "numberString.length()");
		doChecks(5, false, false, false, testExpression, context);
	}
	SECTION("String.length_string2")
	{
		Expression<int> testExpression(&context, "text.length()");
		doChecks(6, false, false, false, testExpression, context);
	}
	SECTION("String.length_nullString")
	{
		Expression<int> testExpression(&context, "nullObject.text.length()");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("String.length_morearg")
	{
		Expression<int> testExpression(&context, "text.length(theInt, aFloat)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("String.length_obj")
	{
		Expression<int> testExpression(&context, "nestedObject.length()");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: String.substring", "[builtins][subString]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_String.substring", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "subStringStaticScope");	

	SECTION("String.substring_cc1")
	{
		Expression<std::string> testExpression(&context, "ToString(11.1f).subString(1, 3)");
		doChecks(std::string("1.1"), false, false, false, testExpression, context);
	}
	SECTION("String.substring_cc2")
	{
		Expression<std::string> testExpression(&context, "ToString(0).subString(toInt(11.0f), 12)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("String.substring_cc3")
	{
		Expression<std::string> testExpression(&context, "\"9\".subString(9, 0)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("String.substring_cc4")
	{
		Expression<std::string> testExpression(&context, "\"test\".subString(0, 4)");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("String.substring_cc5")
	{
		Expression<std::string> testExpression(&context, "\"test\".subString(0, 100)");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("String.substring_cc6")
	{
		Expression<std::string> testExpression(&context, "\"test\".subString(3, 1)");
		doChecks(std::string("t"), false, false, false, testExpression, context);
	}
	SECTION("String.substring_float1")
	{
		Expression<std::string> testExpression(&context, "ToString(aFloat).subString(4, 5)");
		doChecks(std::string("9"), false, false, false, testExpression, context);
	}
	SECTION("String.substring_float2")
	{
		Expression<std::string> testExpression(&context, "ToString(aFloat).subString(toInt(1000.0f), toInt(1.1f))");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("String.substring_int1")
	{
		Expression<std::string> testExpression(&context, "ToString(largeInt).subString(-7, 789)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("String.substring_int2")
	{
		Expression<std::string> testExpression(&context, "ToString(largeInt).subString(-largeInt, 0)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("String.substring_int3")
	{
		Expression<std::string> testExpression(&context, "ToString(largeInt).subString(largeInt, largeInt)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("String.substring_bool")
	{
		Expression<std::string> testExpression(&context, "ToString(aBoolean).subString(0, 1)");
		doChecks(std::string("1"), false, false, false, testExpression, context);
	}
	SECTION("String.substring_stringConst")
	{
		Expression<std::string> testExpression(&context, "\"10\".subString(1, 1)");
		doChecks(std::string("0"), false, false, false, testExpression, context);
	}
	SECTION("String.substring_string")
	{
		Expression<std::string> testExpression(&context, "numberString.subString(3, 2)");
		doChecks(std::string(".4"), false, false, false, testExpression, context);
	}
	SECTION("String.substring_string2")
	{
		Expression<std::string> testExpression(&context, "text.subString(2, 2)");
		doChecks(std::string("ll"), false, false, false, testExpression, context);
	}
	SECTION("String.substring_nullString")
	{
		Expression<std::string> testExpression(&context, "nullObject.text.subString(2, 2)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("String.substring_noarg")
	{
		Expression<std::string> testExpression(&context, "text.subString()");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("String.substring_morearg")
	{
		Expression<std::string> testExpression(&context, "text.subString(theInt, aFloat, 10, false)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("String.substring_obj")
	{
		Expression<std::string> testExpression(&context, "nestedObject.subString(nestedObject, 10, 11)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
}
