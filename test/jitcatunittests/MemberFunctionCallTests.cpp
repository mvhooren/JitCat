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
	CatRuntimeContext context("memberFunctions", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Get float")
	{
		Expression<float> testExpression(&context, "getFloat()");
		doChecks(reflectedObject.getFloat(), false, false, false, testExpression, context);
	}
	SECTION("Get double")
	{
		Expression<double> testExpression(&context, "getDouble()");
		doChecks(reflectedObject.getDouble(), false, false, false, testExpression, context);
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
	SECTION("Get object by value")
	{
		Expression<TestVector4> testExpression(&context, "getTestVector()");
		doChecks(reflectedObject.getTestVector(), false, false, false, testExpression, context);
	}
	SECTION("Get object by value, reference parameter")
	{
		Expression<TestVector4> testExpression(&context, "getTestVector().doAdd(getTestVector())");
		doChecks(reflectedObject.getTestVector().doAdd(reflectedObject.getTestVector()), false, false, false, testExpression, context);
	}
	SECTION("Get object by reference")
	{
		Expression<TestVector4> testExpression(&context, "getTestVectorRef()");
		doChecks(reflectedObject.getTestVectorRef(), false, false, false, testExpression, context);
	}
	SECTION("Get object ptr by reference")
	{
		Expression<TestVector4*> testExpression(&context, "getTestVectorRef()");
		doChecks(&reflectedObject.getTestVectorRef(), false, false, false, testExpression, context);
	}

	SECTION("Const get float")
	{
		Expression<float> testExpression(&context, "getConstantFloat()");
		doChecks(reflectedObject.getConstantFloat(), false, false, false, testExpression, context);
	}
	SECTION("Const get double")
	{
		Expression<double> testExpression(&context, "getConstantDouble()");
		doChecks(reflectedObject.getConstantDouble(), false, false, false, testExpression, context);
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
	SECTION("Const get object by value")
	{
		Expression<TestVector4> testExpression(&context, "getConstTestVector()");
		doChecks(reflectedObject.getConstTestVector(), false, false, false, testExpression, context);
	}
	SECTION("Get object by const reference")
	{
		Expression<TestVector4> testExpression(&context, "getTestVectorConstRef()");
		doChecks(reflectedObject.getTestVectorConstRef(), false, false, false, testExpression, context);
	}
	SECTION("Get object ptr by const reference")
	{
		Expression<TestVector4*> testExpression(&context, "getTestVectorConstRef()");
		doChecks(&reflectedObject.getTestVectorRef(), false, false, false, testExpression, context);
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
		testExpression.getInterpretedValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}
	SECTION("Void function 2")
	{
		Expression<void> testExpression(&context, "checkTheseValues(aBoolean, theInt, text, nestedSelfObject)");
		testExpression.getValue(&context);
		testExpression.getInterpretedValue(&context);
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
	SECTION("object base object value parameters")
	{
		Expression<TestVector4> testExpression(&context, "nestedSelfObject.addVectors(getTestVector(), v2)");
		doChecks<TestVector4>(reflectedObject.nestedSelfObject->addVectors(reflectedObject.nestedSelfObject->getTestVector(), reflectedObject.nestedSelfObject->v2), false, false, false, testExpression, context);
	}
	SECTION("object base object value parameters, automatic dereference")
	{
		Expression<TestVector4> testExpression(&context, "nestedSelfObject.addVectors(getTestVectorPtr(), v2)");
		doChecks<TestVector4>(reflectedObject.nestedSelfObject->addVectors(*reflectedObject.nestedSelfObject->getTestVectorPtr(), reflectedObject.nestedSelfObject->v2), false, false, false, testExpression, context);
	}
	SECTION("Static function call")
	{
		Expression<TestVector4> testExpression(&context, "TestVector4::staticAdd(getTestVectorPtr(), v2, v1)");
		doChecks<TestVector4>(TestVector4::staticAdd(*reflectedObject.getTestVectorPtr(), &reflectedObject.v2, reflectedObject.v1), false, false, false, testExpression, context);
	}

	SECTION("null base float")
	{
		Expression<float> testExpression(&context, "nestedSelfObject.nestedSelfObject.getFloat()");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("null base double")
	{
		Expression<double> testExpression(&context, "nestedSelfObject.nestedSelfObject.getDouble()");
		doChecks(0.0, false, false, false, testExpression, context);
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
		Expression<TestVector4> testExpression(&context, "nestedSelfObject.nestedSelfObject.getTestVector()");
		doChecks<TestVector4>(TestVector4(), false, false, false, testExpression, context);
	}
	SECTION("null base object by value")
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
