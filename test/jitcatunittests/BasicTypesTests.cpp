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


TEST_CASE("Floating Point Tests", "[float][operators]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("floatTests", &errorManager);
	context.addScope(&reflectedObject, true);	
	SECTION("Constant")
	{
		Expression<float> testExpression(&context, "42.0f");
		doChecks(42.0f, false, true, true, testExpression, context);
	}
	SECTION("Constant No Decimal digits")
	{
		Expression<float> testExpression(&context, "42.");
		doChecks(42.0f, false, true, true, testExpression, context);
	}
	SECTION("Constant No Decimal digits 2")
	{
		Expression<float> testExpression(&context, "0.");
		doChecks(0.0f, false, true, true, testExpression, context);
	}
	SECTION("Constant No Integer digits")
	{
		Expression<float> testExpression(&context, ".05");
		doChecks(.05f, false, true, true, testExpression, context);
	}
	SECTION("Constant No Integer digits 2")
	{
		Expression<float> testExpression(&context, ".0");
		doChecks(.0f, false, true, true, testExpression, context);
	}
	SECTION("Constant No Integer digits, with 'f'")
	{
		Expression<float> testExpression(&context, ".05f");
		doChecks(.05f, false, true, true, testExpression, context);
	}
	SECTION("Constant No Integer digits 2, with 'f'")
	{
		Expression<float> testExpression(&context, ".0f");
		doChecks(.0f, false, true, true, testExpression, context);
	}
	SECTION("Constant No Decimal digits, with 'f'")
	{
		Expression<float> testExpression(&context, "42.f");
		doChecks(42.0f, false, true, true, testExpression, context);
	}
	SECTION("Constant with exponent")
	{
		Expression<float> testExpression(&context, "1.0e10");
		doChecks(1e10f, false, true, true, testExpression, context);
	}
	SECTION("Constant with negative exponent")
	{
		Expression<float> testExpression(&context, "1.0e-10");
		doChecks(1e-10f, false, true, true, testExpression, context);
	}
	SECTION("Constant with exponent no decimal point")
	{
		Expression<float> testExpression(&context, "1e10");
		doChecks(1e10f, false, true, true, testExpression, context);
	}
	SECTION("Constant with capital exponent no decimal point")
	{
		Expression<float> testExpression(&context, "1E10");
		doChecks(1e10f, false, true, true, testExpression, context);
	}
	SECTION("Constant with exponent no decimal point and 'f'")
	{
		Expression<float> testExpression(&context, "1e10f");
		doChecks(1e10f, false, true, true, testExpression, context);
	}
	SECTION("Constant with exponent no decimal digits")
	{
		Expression<float> testExpression(&context, "1.e10");
		doChecks(1e10f, false, true, true, testExpression, context);
	}
	SECTION("Constant with exponent no decimal digits, with 'f'")
	{
		Expression<float> testExpression(&context, "1.e10f");
		doChecks(1e10f, false, true, true, testExpression, context);
	}
	SECTION("Constant with exponent and 'f'")
	{
		Expression<float> testExpression(&context, "1.0e10f");
		doChecks(1e10f, false, true, true, testExpression, context);
	}
	SECTION("Constant with exponent and no floating point digits")
	{
		Expression<float> testExpression(&context, ".e10");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Constant with exponent and no exponent digits")
	{
		Expression<float> testExpression(&context, "1.0e");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Constant with exponent and no exponent digits, with 'f'")
	{
		Expression<float> testExpression(&context, "1.0ef");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Constant with negative exponent and no exponent digits")
	{
		Expression<float> testExpression(&context, "1.0e-");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Constant with exponent and extraneous 'f'")
	{
		Expression<float> testExpression(&context, "1.0fe10");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Constant_2")
	{
		Expression<float> testExpression(&context, "42.0");
		doChecks(42.0f, false, true, true, testExpression, context);
	}
	SECTION("Negative Constant")
	{
		Expression<float> testExpression(&context, "-42.0f");
		doChecks(-42.0f, false, true, true, testExpression, context);
	}
	SECTION("Negative Constant_2")
	{
		Expression<float> testExpression(&context, "-42.0");
		doChecks(-42.0f, false, true, true, testExpression, context);
	}
	SECTION("Variable")
	{
		Expression<float> testExpression(&context, "aFloat");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Addition")
	{
		Expression<float> testExpression(&context, "aFloat + 33.3f");
		doChecks(reflectedObject.aFloat + 33.3f, false, false, false, testExpression, context);
	}
	SECTION("Subtraction")
	{
		Expression<float> testExpression(&context, "aFloat - 15.4f");
		doChecks(reflectedObject.aFloat - 15.4f, false, false, false, testExpression, context);
	}
	SECTION("Multiplication")
	{
		Expression<float> testExpression(&context, "aFloat * 22.8f");
		doChecks(reflectedObject.aFloat * 22.8f, false, false, false, testExpression, context);
	}
	SECTION("Division")
	{
		Expression<float> testExpression(&context, "aFloat / 182.0f");
		doChecks(reflectedObject.aFloat / 182.0f, false, false, false, testExpression, context);
	}
	SECTION("Division By Zero")
	{
		Expression<float> testExpression(&context, "aFloat / 0.0f");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Modulo")
	{
		Expression<float> testExpression(&context, "aFloat % 11.5f");
		doChecks(fmodf(reflectedObject.aFloat, 11.5f), false, false, false, testExpression, context);
	}
	SECTION("Modulo_Invalid")
	{
		Expression<float> testExpression(&context, "aFloat % zeroFloat");
		doChecks<float>(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Smaller_1")
	{
		Expression<bool> testExpression(&context, "aFloat < 1000.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Smaller_2")
	{
		Expression<bool> testExpression(&context, "aFloat < 900.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Smaller_3")
	{
		Expression<bool> testExpression(&context, "aFloat < aFloat");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Greater_1")
	{
		Expression<bool> testExpression(&context, "aFloat > 1000.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Greater_2")
	{
		Expression<bool> testExpression(&context, "aFloat > 900.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Greater_3")
	{
		Expression<bool> testExpression(&context, "aFloat > aFloat");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_1")
	{
		Expression<bool> testExpression(&context, "aFloat >= 1000.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_2")
	{
		Expression<bool> testExpression(&context, "aFloat >= 900.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_3")
	{
		Expression<bool> testExpression(&context, "aFloat >= aFloat");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_1")
	{
		Expression<bool> testExpression(&context, "aFloat <= 1000.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_2")
	{
		Expression<bool> testExpression(&context, "aFloat <= 900.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_3")
	{
		Expression<bool> testExpression(&context, "aFloat <= aFloat");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Equals_1")
	{
		Expression<bool> testExpression(&context, "aFloat == 1000.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Equals_2")
	{
		Expression<bool> testExpression(&context, "aFloat == 900.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Equals_3")
	{
		Expression<bool> testExpression(&context, "aFloat == 999.9f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_1")
	{
		Expression<bool> testExpression(&context, "aFloat != 1000.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_2")
	{
		Expression<bool> testExpression(&context, "aFloat != 900.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_3")
	{
		Expression<bool> testExpression(&context, "aFloat != 999.9f");
		doChecks(false, false, false, false, testExpression, context);
	}
}


TEST_CASE("Integer Tests", "[int][operators]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("intTests", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Constant")
	{
		Expression<int> testExpression(&context, "42");
		doChecks(42, false, true, true, testExpression, context);
	}
	SECTION("Negative Constant")
	{
		Expression<int> testExpression(&context, "-42");
		doChecks(-42, false, true, true, testExpression, context);
	}
	SECTION("Variable")
	{
		Expression<int> testExpression(&context, "theInt");
		doChecks(reflectedObject.theInt, false, false, false, testExpression, context);
	}
	SECTION("Addition")
	{
		Expression<int> testExpression(&context, "theInt + 33");
		doChecks(reflectedObject.theInt + 33, false, false, false, testExpression, context);
	}
	SECTION("Subtraction")
	{
		Expression<int> testExpression(&context, "theInt - 15");
		doChecks(reflectedObject.theInt - 15, false, false, false, testExpression, context);
	}
	SECTION("Multiplication")
	{
		Expression<int> testExpression(&context, "theInt * 22");
		doChecks(reflectedObject.theInt * 22, false, false, false, testExpression, context);
	}
	SECTION("Division")
	{
		Expression<int> testExpression(&context, "theInt / 12");
		doChecks(reflectedObject.theInt / 12, false, false, false, testExpression, context);
	}
	SECTION("Division By Zero")
	{
		Expression<int> testExpression(&context, "theInt / 0");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Modulo")
	{
		Expression<int> testExpression(&context, "theInt % 11");
		doChecks(reflectedObject.theInt % 11, false, false, false, testExpression, context);
	}
	SECTION("Modulo_Invalid")
	{
		Expression<int> testExpression(&context, "theInt % 0");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Smaller_1")
	{
		Expression<bool> testExpression(&context, "theInt < 1000");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Smaller_2")
	{
		Expression<bool> testExpression(&context, "theInt < 41");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Smaller_3")
	{
		Expression<bool> testExpression(&context, "theInt < theInt");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Greater_1")
	{
		Expression<bool> testExpression(&context, "theInt > 1000");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Greater_2")
	{
		Expression<bool> testExpression(&context, "theInt > 41");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Greater_3")
	{
		Expression<bool> testExpression(&context, "theInt > theInt");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_1")
	{
		Expression<bool> testExpression(&context, "theInt >= 1000");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_2")
	{
		Expression<bool> testExpression(&context, "theInt >= 40");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_3")
	{
		Expression<bool> testExpression(&context, "theInt >= theInt");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_1")
	{
		Expression<bool> testExpression(&context, "theInt <= 1000");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_2")
	{
		Expression<bool> testExpression(&context, "theInt <= 40");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_3")
	{
		Expression<bool> testExpression(&context, "theInt <= theInt");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Equals_1")
	{
		Expression<bool> testExpression(&context, "theInt == 1000");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Equals_2")
	{
		Expression<bool> testExpression(&context, "theInt == 0");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Equals_3")
	{
		Expression<bool> testExpression(&context, "theInt == 42");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_1")
	{
		Expression<bool> testExpression(&context, "theInt != 1000");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_2")
	{
		Expression<bool> testExpression(&context, "theInt != 40");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_3")
	{
		Expression<bool> testExpression(&context, "theInt != 42");
		doChecks(false, false, false, false, testExpression, context);
	}
}


TEST_CASE("Boolean Tests", "[bool][operators]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("intTests", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("True Constant")
	{
		Expression<bool> testExpression(&context, "true");
		doChecks(true, false, true, true, testExpression, context);
	}
	SECTION("False Constant")
	{
		Expression<bool> testExpression(&context, "false");
		doChecks(false, false, true, true, testExpression, context);
	}
	SECTION("Constant Operator And")
	{
		Expression<bool> testExpression(&context, "false && true");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("Constant Operator Or")
	{
		Expression<bool> testExpression(&context, "false || true");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Constant Operator Not")
	{
		Expression<bool> testExpression(&context, "!false");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Constant Operator Equals")
	{
		Expression<bool> testExpression(&context, "false == false");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Constant Operator Not Equals")
	{
		Expression<bool> testExpression(&context, "true != false");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Variable")
	{
		Expression<bool> testExpression(&context, "aBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Not")
	{
		Expression<bool> testExpression(&context, "!aBoolean");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Not")
	{
		Expression<bool> testExpression(&context, "!no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator And")
	{
		Expression<bool> testExpression(&context, "aBoolean && no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator And")
	{
		Expression<bool> testExpression(&context, "aBoolean && !no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator And")
	{
		Expression<bool> testExpression(&context, "!aBoolean && no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Or")
	{
		Expression<bool> testExpression(&context, "aBoolean || no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Or")
	{
		Expression<bool> testExpression(&context, "!aBoolean || !no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Or")
	{
		Expression<bool> testExpression(&context, "!aBoolean || no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Equals")
	{
		Expression<bool> testExpression(&context, "aBoolean == no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Equals")
	{
		Expression<bool> testExpression(&context, "aBoolean == !no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Equals")
	{
		Expression<bool> testExpression(&context, "!aBoolean == no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator NotEquals")
	{
		Expression<bool> testExpression(&context, "aBoolean != no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator NotEquals")
	{
		Expression<bool> testExpression(&context, "aBoolean != !no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator NotEquals")
	{
		Expression<bool> testExpression(&context, "!aBoolean != no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Addition")
	{
		Expression<bool> testExpression(&context, "aBoolean + 33");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Subtraction")
	{
		Expression<bool> testExpression(&context, "aBoolean - 15");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Multiplication")
	{
		Expression<bool> testExpression(&context, "aBoolean * 22");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Division")
	{
		Expression<bool> testExpression(&context, "aBoolean / 12");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Division By Zero")
	{
		Expression<bool> testExpression(&context, "aBoolean / 0");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Modulo")
	{
		Expression<bool> testExpression(&context, "aBoolean % 11");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Modulo_Invalid")
	{
		Expression<bool> testExpression(&context, "aBoolean % 0");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Smaller_1")
	{
		Expression<bool> testExpression(&context, "aBoolean < 1000");
		doChecks(true, true, false, false, testExpression, context);
	}
	SECTION("Smaller_2")
	{
		Expression<bool> testExpression(&context, "aBoolean < 41");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Smaller_3")
	{
		Expression<bool> testExpression(&context, "aBoolean < theInt");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Greater_1")
	{
		Expression<bool> testExpression(&context, "aBoolean > 1000");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Greater_2")
	{
		Expression<bool> testExpression(&context, "aBoolean > 41");
		doChecks(true, true, false, false, testExpression, context);
	}
	SECTION("Greater_3")
	{
		Expression<bool> testExpression(&context, "aBoolean > theInt");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_1")
	{
		Expression<bool> testExpression(&context, "aBoolean >= 1000");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_2")
	{
		Expression<bool> testExpression(&context, "aBoolean >= 40");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_3")
	{
		Expression<bool> testExpression(&context, "aBoolean >= theInt");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_1")
	{
		Expression<bool> testExpression(&context, "aBoolean <= 1000");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_2")
	{
		Expression<bool> testExpression(&context, "aBoolean <= 40");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_3")
	{
		Expression<bool> testExpression(&context, "aBoolean <= theInt");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Equals_1")
	{
		Expression<bool> testExpression(&context, "aBoolean == 1000");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Equals_2")
	{
		Expression<bool> testExpression(&context, "aBoolean == 0");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Equals_3")
	{
		Expression<bool> testExpression(&context, "aBoolean == 42");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("NotEquals_1")
	{
		Expression<bool> testExpression(&context, "aBoolean != 1000");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("NotEquals_2")
	{
		Expression<bool> testExpression(&context, "aBoolean != 40");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("NotEquals_3")
	{
		Expression<bool> testExpression(&context, "aBoolean != 42");
		doChecks(false, true, false, false, testExpression, context);
	}
}


TEST_CASE("String Tests", "[float][operators]")
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("floatTests", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Constant")
	{
		Expression<std::string> testExpression(&context, "\"hello\"");
		doChecks(std::string("hello"), false, true, true, testExpression, context);
	}
	SECTION("Constant addition int")
	{
		Expression<std::string> testExpression(&context, "\"hello\" + 2");
		doChecks(std::string("hello2"), false, true, false , testExpression, context);
	}
	SECTION("Constant addition float")
	{
		Expression<std::string> testExpression(&context, "\"hello\" + 2.1f");
		doChecks(std::string("hello2.1"), false, true, false, testExpression, context);
	}
	SECTION("Constant addition bool")
	{
		Expression<std::string> testExpression(&context, "\"hello\" + false");
		doChecks(std::string("hello0"), false, true, false, testExpression, context);
	}
	SECTION("Constant addition string")
	{
		Expression<std::string> testExpression(&context, "\"hello\" + \"test\"");
		doChecks(std::string("hellotest"), false, true, false, testExpression, context);
	}

	SECTION("Constant reverse addition int")
	{
		Expression<std::string> testExpression(&context, "2 + \"hello\"");
		doChecks(std::string("2hello"), false, true, false, testExpression, context);
	}
	SECTION("Constant reverse addition float")
	{
		Expression<std::string> testExpression(&context, "2.1f + \"hello\"");
		doChecks(std::string("2.1hello"), false, true, false, testExpression, context);
	}
	SECTION("Constant reverse addition bool")
	{
		Expression<std::string> testExpression(&context, "true + \"hello\"");
		doChecks(std::string("1hello"), false, true, false, testExpression, context);
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
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("Constant comparison true")
	{
		Expression<bool> testExpression(&context, "\"hello\" == \"hello\"");
		doChecks(true, false, true, false, testExpression, context);
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
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Constant not-comparison false")
	{
		Expression<bool> testExpression(&context, "\"hello\" != \"hello\"");
		doChecks(false, false, true, false, testExpression, context);
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
