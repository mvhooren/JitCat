/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Configuration.h"
#include "jitcat/TypeInfo.h"
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
	context.addScope(&reflectedObject, true);

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
	context.addScope(&reflectedObject, true);	

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


TEST_CASE("Builtin functions test: find", "[builtins][findInString]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_FindInString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("FindInString_cc1")
	{
		Expression<int> testExpression(&context, "ToString(11.1f).find(ToString(1))");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("FindInString_cc2")
	{
		Expression<int> testExpression(&context, "ToString(0).find(ToString(11.0f))");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("FindInString_cc3")
	{
		Expression<int> testExpression(&context, "ToString(9).find(ToString(-9))");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("FindInString_cc4")
	{
		Expression<int> testExpression(&context, "\"test\".find(\"t\")");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("FindInString_cc5")
	{
		Expression<int> testExpression(&context, "\"test\".find(\"est\")");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("FindInString_cc6")
	{
		Expression<int> testExpression(&context, "\"test\".find(\"xxx\")");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("FindInString_float1")
	{
		Expression<int> testExpression(&context, "ToString(aFloat).find(ToString(9))");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("FindInString_float2")
	{
		Expression<int> testExpression(&context, "ToString(aFloat).find(ToString(1000.0f))");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("FindInString_int1")
	{
		Expression<int> testExpression(&context, "ToString(largeInt).find(ToString(7))");
		doChecks(6, false, false, false, testExpression, context);
	}
	SECTION("FindInString_int2")
	{
		Expression<int> testExpression(&context, "ToString(largeInt).find(ToString(-largeInt))");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("FindInString_int3")
	{
		Expression<int> testExpression(&context, "ToString(largeInt).find(ToString(largeInt))");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("FindInString_bool")
	{
		Expression<int> testExpression(&context, "ToString(aBoolean).find(\"1\")");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("FindInString_stringConst")
	{
		Expression<int> testExpression(&context, "\"10\".find(ToString(10))");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("FindInString_string")
	{
		Expression<int> testExpression(&context, "numberString.find(\".4\")");
		doChecks(3, false, false, false, testExpression, context);
	}
	SECTION("FindInString_string2")
	{
		Expression<int> testExpression(&context, "text.find(\"ll\")");
		doChecks(2, false, false, false, testExpression, context);
	}
	SECTION("FindInString_nullString")
	{
		Expression<int> testExpression(&context, "nullObject.text.find(\"ll\")");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("FindInString_nullArg")
	{
		Expression<int> testExpression(&context, "text.find(nullObject.text)");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("FindInString_noarg")
	{
		Expression<int> testExpression(&context, "text.find()");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("FindInString_morearg")
	{
		Expression<int> testExpression(&context, "text.find(theInt, aFloat, 10)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("FindInString_obj")
	{
		Expression<int> testExpression(&context, "text.find(nestedObject, 10)");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ReplaceInString", "[builtins][replaceInString]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ReplaceInString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ReplaceInString_cc1")
	{
		Expression<std::string> testExpression(&context, "replaceInString(11.1f, 1, 2)");
		doChecks(std::string("22.2"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_cc2")
	{
		Expression<std::string> testExpression(&context, "replaceInString(0, 11.0f, 12)");
		doChecks(std::string("0"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_cc3")
	{
		Expression<std::string> testExpression(&context, "replaceInString(\"9\", 9, \"7\")");
		doChecks(std::string("7"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_cc4")
	{
		Expression<std::string> testExpression(&context, "replaceInString(\"test\", \"t\", \"tt\")");
		doChecks(std::string("ttestt"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_cc5")
	{
		Expression<std::string> testExpression(&context, "replaceInString(\"test\", \"est\", \"om\")");
		doChecks(std::string("tom"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_cc6")
	{
		Expression<std::string> testExpression(&context, "replaceInString(\"test\", \"xxx\", false)");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_float1")
	{
		Expression<std::string> testExpression(&context, "replaceInString(aFloat, 9, true)");
		doChecks(std::string("111.1"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_float2")
	{
		Expression<std::string> testExpression(&context, "replaceInString(aFloat, 1000.0f, \"test\")");
		doChecks(std::string("999.9"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_int1")
	{
		Expression<std::string> testExpression(&context, "replaceInString(largeInt, 7, 789)");
		doChecks(std::string("123456789"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_int2")
	{
		Expression<std::string> testExpression(&context, "replaceInString(largeInt, -largeInt, 0)");
		doChecks(std::string("1234567"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_int3")
	{
		Expression<std::string> testExpression(&context, "replaceInString(largeInt, largeInt, largeInt)");
		doChecks(std::string("1234567"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_bool")
	{
		Expression<std::string> testExpression(&context, "replaceInString(aBoolean, \"1\", false)");
		doChecks(std::string("0"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_stringConst")
	{
		Expression<std::string> testExpression(&context, "replaceInString(\"10\", 10, \"42\")");
		doChecks(std::string("42"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_string")
	{
		Expression<std::string> testExpression(&context, "replaceInString(numberString, \".4\", 4)");
		doChecks(std::string("1234"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_string2")
	{
		Expression<std::string> testExpression(&context, "replaceInString(text, \"ll\", text)");
		doChecks(std::string("HeHello!o!"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_noarg")
	{
		Expression<std::string> testExpression(&context, "replaceInString()");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_morearg")
	{
		Expression<std::string> testExpression(&context, "replaceInString(theInt, aFloat, 10, false)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_obj")
	{
		Expression<std::string> testExpression(&context, "replaceInString(nestedObject, 10, false)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: StringLength", "[builtins][stringLength]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_StringLength", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("StringLength_cc1")
	{
		Expression<int> testExpression(&context, "stringLength(11.1f)");
		doChecks(4, false, true, false, testExpression, context);
	}
	SECTION("StringLength_cc2")
	{
		Expression<int> testExpression(&context, "stringLength(0)");
		doChecks(1, false, true, false, testExpression, context);
	}
	SECTION("StringLength_cc3")
	{
		Expression<int> testExpression(&context, "stringLength(9)");
		doChecks(1, false, true, false, testExpression, context);
	}
	SECTION("StringLength_cc4")
	{
		Expression<int> testExpression(&context, "stringLength(\"test\")");
		doChecks(4, false, true, false, testExpression, context);
	}
	SECTION("StringLength_cc5")
	{
		Expression<int> testExpression(&context, "stringLength(\"\")");
		doChecks(0, false, true, false, testExpression, context);
	}
	SECTION("StringLength_float1")
	{
		Expression<int> testExpression(&context, "stringLength(aFloat)");
		doChecks(5, false, false, false, testExpression, context);
	}
	SECTION("StringLength_float2")
	{
		Expression<int> testExpression(&context, "stringLength(zeroFloat)");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("StringLength_int1")
	{
		Expression<int> testExpression(&context, "stringLength(largeInt)");
		doChecks(7, false, false, false, testExpression, context);
	}
	SECTION("StringLength_int2")
	{
		Expression<int> testExpression(&context, "stringLength(-largeInt)");
		doChecks(8, false, false, false, testExpression, context);
	}
	SECTION("StringLength_bool")
	{
		Expression<int> testExpression(&context, "stringLength(aBoolean)");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("StringLength_stringConst")
	{
		Expression<int> testExpression(&context, "stringLength(\"10\")");
		doChecks(2, false, true, false, testExpression, context);
	}
	SECTION("StringLength_string")
	{
		Expression<int> testExpression(&context, "stringLength(numberString)");
		doChecks(5, false, false, false, testExpression, context);
	}
	SECTION("StringLength_string2")
	{
		Expression<int> testExpression(&context, "stringLength(text)");
		doChecks(6, false, false, false, testExpression, context);
	}
	SECTION("StringLength_noarg")
	{
		Expression<int> testExpression(&context, "stringLength()");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("StringLength_morearg")
	{
		Expression<int> testExpression(&context, "stringLength(theInt, aFloat)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("StringLength_obj")
	{
		Expression<int> testExpression(&context, "stringLength(nestedObject)");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: SubString", "[builtins][subString]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_SubString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("SubString_cc1")
	{
		Expression<std::string> testExpression(&context, "subString(11.1f, 1, 3)");
		doChecks(std::string("1.1"), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc2")
	{
		Expression<std::string> testExpression(&context, "subString(0, 11.0f, 12)");
		doChecks(std::string(""), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc3")
	{
		Expression<std::string> testExpression(&context, "subString(\"9\", 9, 0)");
		doChecks(std::string(""), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc4")
	{
		Expression<std::string> testExpression(&context, "subString(\"test\", 0, 4)");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc5")
	{
		Expression<std::string> testExpression(&context, "subString(\"test\", 0, 100)");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc6")
	{
		Expression<std::string> testExpression(&context, "subString(\"test\", 3, 1)");
		doChecks(std::string("t"), false, true, false, testExpression, context);
	}
	SECTION("SubString_float1")
	{
		Expression<std::string> testExpression(&context, "subString(aFloat, 4, 5)");
		doChecks(std::string("9"), false, false, false, testExpression, context);
	}
	SECTION("SubString_float2")
	{
		Expression<std::string> testExpression(&context, "subString(aFloat, 1000.0f, 1.1f)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("SubString_int1")
	{
		Expression<std::string> testExpression(&context, "subString(largeInt, -7, 789)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("SubString_int2")
	{
		Expression<std::string> testExpression(&context, "subString(largeInt, -largeInt, 0)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("SubString_int3")
	{
		Expression<std::string> testExpression(&context, "subString(largeInt, largeInt, largeInt)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("SubString_bool")
	{
		Expression<std::string> testExpression(&context, "subString(aBoolean, 0, 1)");
		doChecks(std::string("1"), false, false, false, testExpression, context);
	}
	SECTION("SubString_stringConst")
	{
		Expression<std::string> testExpression(&context, "subString(\"10\", 1, 1)");
		doChecks(std::string("0"), false, true, false, testExpression, context);
	}
	SECTION("SubString_string")
	{
		Expression<std::string> testExpression(&context, "subString(numberString, 3, 2)");
		doChecks(std::string(".4"), false, false, false, testExpression, context);
	}
	SECTION("SubString_string2")
	{
		Expression<std::string> testExpression(&context, "subString(text, 2, 2)");
		doChecks(std::string("ll"), false, false, false, testExpression, context);
	}
	SECTION("SubString_noarg")
	{
		Expression<std::string> testExpression(&context, "subString()");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("SubString_morearg")
	{
		Expression<std::string> testExpression(&context, "subString(theInt, aFloat, 10, false)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("SubString_obj")
	{
		Expression<std::string> testExpression(&context, "subString(nestedObject, 10, 11)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
}
