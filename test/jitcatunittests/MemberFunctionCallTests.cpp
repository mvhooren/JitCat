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


TEST_CASE("Member Functions", "[memberfunctions]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Select", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Get float")
	{
		Expression<float> testExpression(&context, "getFloat()");
		doChecks(reflectedObject.getFloat(), false, false, false, testExpression, context);
	}
	SECTION("Get int")
	{
		Expression<int> testExpression(&context, "getint()");
		doChecks(reflectedObject.getInt(), false, false, false, testExpression, context);
	}
	SECTION("Get bool")
	{
		Expression<bool> testExpression(&context, "getBoolean()");
		doChecks(reflectedObject.getBoolean(), false, false, false, testExpression, context);
	}
	SECTION("Get string")
	{
		Expression<std::string> testExpression(&context, "getString()");
		doChecks(reflectedObject.getString(), false, false, false, testExpression, context);
	}
	SECTION("Get object")
	{
		Expression<ReflectedObject*> testExpression(&context, "getObject()");
		doChecks(reflectedObject.getObject(), false, false, false, testExpression, context);
	}

	SECTION("Const get float")
	{
		Expression<float> testExpression(&context, "getConstantFloat()");
		doChecks(reflectedObject.getConstantFloat(), false, false, false, testExpression, context);
	}
	SECTION("Const get int")
	{
		Expression<int> testExpression(&context, "getConstInt()");
		doChecks(reflectedObject.getConstInt(), false, false, false, testExpression, context);
	}
	SECTION("Const get bool")
	{
		Expression<bool> testExpression(&context, "getConstBool()");
		doChecks(reflectedObject.getConstBool(), false, false, false, testExpression, context);
	}
	SECTION("Const get string")
	{
		Expression<std::string> testExpression(&context, "getConstString()");
		doChecks(reflectedObject.getConstString(), false, false, false, testExpression, context);
	}
	SECTION("Const get object")
	{
		Expression<ReflectedObject*> testExpression(&context, "getConstObject()");
		doChecks(reflectedObject.getConstObject(), false, false, false, testExpression, context);
	}
	SECTION("Void function")
	{
		Expression<void> testExpression(&context, "doSomething()");
		testExpression.getValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}
	SECTION("Const void function")
	{
		Expression<void> testExpression(&context, "doSomethingConst()");
		testExpression.getValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}
	SECTION("Void function 2")
	{
		Expression<void> testExpression(&context, "checkTheseValues(aBoolean, theInt, text, nestedSelfObject)");
		testExpression.getValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}
	SECTION("string function 1")
	{
		Expression<std::string> testExpression(&context, "returnThisString(text)");
		doChecks(reflectedObject.text, false, false, false, testExpression, context);
	}
	SECTION("string function 1")
	{
		Expression<std::string> testExpression(&context, "addToString(text, aFloat)");
		doChecks(Tools::append(reflectedObject.text, reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("object function 1")
	{
		Expression<ReflectedObject*> testExpression(&context, "getThisObject(nestedSelfObject)");
		doChecks(reflectedObject.nestedSelfObject, false, false, false, testExpression, context);
	}

	SECTION("object base float")
	{
		Expression<float> testExpression(&context, "nestedSelfObject.getFloat()");
		doChecks(reflectedObject.nestedSelfObject->getFloat(), false, false, false, testExpression, context);
	}
	SECTION("object base int")
	{
		Expression<int> testExpression(&context, "nestedSelfObject.getInt()");
		doChecks(reflectedObject.nestedSelfObject->getInt(), false, false, false, testExpression, context);
	}
	SECTION("object base boolean")
	{
		Expression<bool> testExpression(&context, "nestedSelfObject.getBoolean()");
		doChecks(reflectedObject.nestedSelfObject->getBoolean(), false, false, false, testExpression, context);
	}
	SECTION("object base string")
	{
		Expression<std::string> testExpression(&context, "nestedSelfObject.getString()");
		doChecks(reflectedObject.nestedSelfObject->getString(), false, false, false, testExpression, context);
	}
	SECTION("object base object")
	{
		Expression<ReflectedObject*> testExpression(&context, "nestedSelfObject.getObject()");
		doChecks<ReflectedObject*>(reflectedObject.nestedSelfObject->getObject(), false, false, false, testExpression, context);
	}
	SECTION("object base void")
	{
		Expression<void> testExpression(&context, "nestedSelfObject.doSomething()");
		testExpression.getValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}

	SECTION("null base float")
	{
		Expression<float> testExpression(&context, "nestedSelfObject.nestedSelfObject.getFloat()");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("null base int")
	{
		Expression<int> testExpression(&context, "nestedSelfObject.nestedSelfObject.getInt()");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("null base boolean")
	{
		Expression<bool> testExpression(&context, "nestedSelfObject.nestedSelfObject.getBoolean()");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("null base string")
	{
		Expression<std::string> testExpression(&context, "nestedSelfObject.nestedSelfObject.getString()");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("null base object")
	{
		Expression<ReflectedObject*> testExpression(&context, "nestedSelfObject.nestedSelfObject.getObject()");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("null base void")
	{
		Expression<void> testExpression(&context, "nestedSelfObject.nestedSelfObject.doSomething()");
		testExpression.getValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}
}
