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


//This tests static function calls
TEST_CASE("Static Functions", "[staticfunctions]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("staticfunctions_tests", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "staticFunctionsStaticScope");	

	SECTION("Global scope get float")
	{
		Expression<float> testExpression(&context, "getStaticFloat()");
		doChecks(ReflectedObject::getStaticFloat(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get double")
	{
		Expression<double> testExpression(&context, "getStaticDouble()");
		doChecks(ReflectedObject::getStaticDouble(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get int")
	{
		Expression<int> testExpression(&context, "getStaticInt()");
		doChecks(ReflectedObject::getStaticInt(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get bool")
	{
		Expression<bool> testExpression(&context, "getStaticBool()");
		doChecks(ReflectedObject::getStaticBool(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get string")
	{
		Expression<std::string> testExpression(&context, "getStaticString()");
		doChecks(ReflectedObject::getStaticString(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get object")
	{
		Expression<TestVector4> testExpression(&context, "getStaticObject()");
		doChecks(ReflectedObject::getStaticObject(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get const object")
	{
		Expression<TestVector4> testExpression(&context, "getStaticConstObject()");
		doChecks(ReflectedObject::getStaticConstObject(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get object reference")
	{
		Expression<TestVector4> testExpression(&context, "getStaticObjectRef()");
		doChecks(ReflectedObject::getStaticObjectRef(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get object reference to ptr")
	{
		Expression<TestVector4*> testExpression(&context, "getStaticObjectRef()");
		doChecks(&ReflectedObject::getStaticObjectRef(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get object const reference")
	{
		Expression<TestVector4> testExpression(&context, "getStaticObjectConstRef()");
		doChecks(ReflectedObject::getStaticObjectConstRef(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get object const reference to ptr")
	{
		Expression<const TestVector4*> testExpression(&context, "getStaticObjectConstRef()");
		doChecks(&ReflectedObject::getStaticObjectConstRef(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get object ptr")
	{
		Expression<TestVector4*> testExpression(&context, "getStaticObjectPtr()");
		doChecks(ReflectedObject::getStaticObjectPtr(), false, false, false, testExpression, context);
	}
	SECTION("Global scope get object ptr to value")
	{
		Expression<TestVector4> testExpression(&context, "getStaticObjectPtr()");
		doChecks(*ReflectedObject::getStaticObjectPtr(), false, false, false, testExpression, context);
	}

	SECTION("Explicit static scope get float")
	{
		Expression<float> testExpression(&context, "ReflectedObject::getStaticFloat()");
		doChecks(ReflectedObject::getStaticFloat(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get double")
	{
		Expression<double> testExpression(&context, "ReflectedObject::getStaticDouble()");
		doChecks(ReflectedObject::getStaticDouble(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get int")
	{
		Expression<int> testExpression(&context, "ReflectedObject::getStaticInt()");
		doChecks(ReflectedObject::getStaticInt(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get bool")
	{
		Expression<bool> testExpression(&context, "ReflectedObject::getStaticBool()");
		doChecks(ReflectedObject::getStaticBool(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get string")
	{
		Expression<std::string> testExpression(&context, "ReflectedObject::getStaticString()");
		doChecks(ReflectedObject::getStaticString(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get object")
	{
		Expression<TestVector4> testExpression(&context, "ReflectedObject::getStaticObject()");
		doChecks(ReflectedObject::getStaticObject(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get const object")
	{
		Expression<TestVector4> testExpression(&context, "ReflectedObject::getStaticConstObject()");
		doChecks(ReflectedObject::getStaticConstObject(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get object reference")
	{
		Expression<TestVector4> testExpression(&context, "ReflectedObject::getStaticObjectRef()");
		doChecks(ReflectedObject::getStaticObjectRef(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get object reference to ptr")
	{
		Expression<TestVector4*> testExpression(&context, "ReflectedObject::getStaticObjectRef()");
		doChecks(&ReflectedObject::getStaticObjectRef(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get object const reference")
	{
		Expression<TestVector4> testExpression(&context, "ReflectedObject::getStaticObjectConstRef()");
		doChecks(ReflectedObject::getStaticObjectConstRef(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get object const reference to ptr")
	{
		Expression<const TestVector4*> testExpression(&context, "ReflectedObject::getStaticObjectConstRef()");
		doChecks(&ReflectedObject::getStaticObjectConstRef(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get object ptr")
	{
		Expression<TestVector4*> testExpression(&context, "ReflectedObject::getStaticObjectPtr()");
		doChecks(ReflectedObject::getStaticObjectPtr(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope get object ptr to value")
	{
		Expression<TestVector4> testExpression(&context, "ReflectedObject::getStaticObjectPtr()");
		doChecks(*ReflectedObject::getStaticObjectPtr(), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope invalid function name")
	{
		Expression<TestVector4> testExpression(&context, "ReflectedObject::noFunction()");
		doChecks(*ReflectedObject::getStaticObjectPtr(), true, false, false, testExpression, context);
	}
	SECTION("Explicit static scope invalid scope name")
	{
		Expression<TestVector4> testExpression(&context, "NoObject::getStaticObjectPtr()");
		doChecks(*ReflectedObject::getStaticObjectPtr(), true, false, false, testExpression, context);
	}

	SECTION("Explicit static scope function with parameters")
	{
		Expression<TestVector4> testExpression(&context, "TestVector4::staticAdd(getTestVector(), v1, getTestVectorPtr())");
		doChecks(TestVector4::staticAdd(reflectedObject.getTestVector(), &reflectedObject.v1, *reflectedObject.getTestVectorPtr()), false, false, false, testExpression, context);
	}
	SECTION("Explicit static scope function with invalid parameter")
	{
		Expression<TestVector4> testExpression(&context, "TestVector4::staticAdd(getTestVector(), 1, getTestVectorPtr())");
		doChecks(TestVector4::staticAdd(reflectedObject.getTestVector(), &reflectedObject.v1, *reflectedObject.getTestVectorPtr()), true, false, false, testExpression, context);
	}
}