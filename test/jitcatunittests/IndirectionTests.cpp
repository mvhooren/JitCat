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


TEST_CASE("Indirection tests", "[indirection]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("indirection", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("NestedSelfObject String")
	{
		Expression<std::string> testExpression(&context, "nestedSelfObject.numberString");
		doChecks(std::string("123.4"), false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Int")
	{
		Expression<int> testExpression(&context, "nestedSelfObject.theInt");
		doChecks(42, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Boolean")
	{
		Expression<bool> testExpression(&context, "nestedSelfObject.aBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Float")
	{
		Expression<float> testExpression(&context, "nestedSelfObject.aFloat");
		doChecks(999.9f, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Double")
	{
		Expression<double> testExpression(&context, "nestedSelfObject.aDouble");
		doChecks(reflectedObject.nestedSelfObject->aDouble, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Object")
	{
		Expression<ReflectedObject*> testExpression(&context, "nestedSelfObject.nestedSelfObject");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Object by value")
	{
		Expression<TestVector4> testExpression(&context, "nestedSelfObject.v1");
		doChecks<TestVector4>(reflectedObject.nestedSelfObject->v1, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Vector")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nestedSelfObject.reflectableObjectsVector[0]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Map")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nestedSelfObject.reflectableObjectsMap[\"one\"]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Error")
	{
		Expression<int> testExpression(&context, "nestedSelfObject.notAnInt");
		doChecks(0, true, false, false, testExpression, context);
	}

	SECTION("nestedObject String")
	{
		Expression<std::string> testExpression(&context, "nestedObject.someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("nestedObject Int")
	{
		Expression<int> testExpression(&context, "nestedObject.someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("nestedObject Boolean")
	{
		Expression<bool> testExpression(&context, "nestedObject.someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("nestedObject Float")
	{
		Expression<float> testExpression(&context, "nestedObject.someFloat");
		doChecks(1.1f, false, false, false, testExpression, context);
	}
	SECTION("nestedObject Double")
	{
		Expression<double> testExpression(&context, "nestedObject.someDouble");
		doChecks(1.1, false, false, false, testExpression, context);
	}
	SECTION("nestedObject Object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nestedObject.nullObject");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nestedObject Object by value")
	{
		//Change something to make sure nestedObject is not equal to its default constructed value.
		reflectedObject.nestedObject.someV4.z = 42.0f;
		Expression<NestedReflectedObject> testExpression(&context, "nestedObject");
		doChecks<NestedReflectedObject>(reflectedObject.nestedObject, false, false, false, testExpression, context);
		reflectedObject.nestedObject.someV4.z = 4.0f;
	}
	SECTION("nestedObject Error")
	{
		Expression<int> testExpression(&context, "nestedObject.notAnInt");
		doChecks(0, true, false, false, testExpression, context);
	}

	SECTION("nestedObjectPointer String")
	{
		Expression<std::string> testExpression(&context, "nestedObjectPointer.someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Int")
	{
		Expression<int> testExpression(&context, "nestedObjectPointer.someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Boolean")
	{
		Expression<bool> testExpression(&context, "nestedObjectPointer.someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Float")
	{
		Expression<float> testExpression(&context, "nestedObjectPointer.someFloat");
		doChecks(1.1f, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Float")
	{
		Expression<double> testExpression(&context, "nestedObjectPointer.someDouble");
		doChecks(1.1, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nestedObjectPointer.nullObject");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Object by value")
	{
		Expression<TestVector4> testExpression(&context, "nestedObjectPointer.someV4");
		doChecks<TestVector4>(reflectedObject.nestedObjectPointer->someV4, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Error")
	{
		Expression<int> testExpression(&context, "nestedObjectPointer.notAnInt");
		doChecks(0, true, false, false, testExpression, context);
	}

	SECTION("nestedObjectUniquePointer String")
	{
		Expression<std::string> testExpression(&context, "nestedObjectUniquePointer.someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Int")
	{
		Expression<int> testExpression(&context, "nestedObjectUniquePointer.someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Boolean")
	{
		Expression<bool> testExpression(&context, "nestedObjectUniquePointer.someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Float")
	{
		Expression<float> testExpression(&context, "nestedObjectUniquePointer.someFloat");
		doChecks(1.1f, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Double")
	{
		Expression<double> testExpression(&context, "nestedObjectUniquePointer.someDouble");
		doChecks(1.1, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nestedObjectUniquePointer.nullObject");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Object by value")
	{
		Expression<TestVector4> testExpression(&context, "nestedObjectUniquePointer.someV4");
		doChecks<TestVector4>(reflectedObject.nestedObjectUniquePointer->someV4, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Error")
	{
		Expression<int> testExpression(&context, "nestedObjectUniquePointer.notAnInt");
		doChecks(0, true, false, false, testExpression, context);
	}

	SECTION("nested null object String")
	{
		Expression<std::string> testExpression(&context, "nullObject.text");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("nested null object Int")
	{
		Expression<int> testExpression(&context, "nullObject.theInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("nested null object Boolean")
	{
		Expression<bool> testExpression(&context, "nullObject.no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("nested null object Float")
	{
		Expression<float> testExpression(&context, "nullObject.aFloat");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("nested null object Double")
	{
		Expression<double> testExpression(&context, "nullObject.aDouble");
		doChecks(0.0, false, false, false, testExpression, context);
	}
	SECTION("nested null object ObjectPtr")
	{
		Expression<ReflectedObject*> testExpression(&context, "nullObject.nullObject");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nested null object Object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.nestedObject");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nested null Object, Object by value")
	{
		Expression<TestVector4> testExpression(&context, "nullObject.v1");
		doChecks<TestVector4>(TestVector4(), false, false, false, testExpression, context);
	}
	SECTION("nested null object UniquePtr")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.nestedObjectUniquePointer");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nested null object Error")
	{
		Expression<int> testExpression(&context, "nullObject.notAnInt");
		doChecks(0, true, false, false, testExpression, context);
	}

	SECTION("badBaseObject Error")
	{
		Expression<int> testExpression(&context, "notAPointer.theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing base object Error")
	{
		Expression<int> testExpression(&context, ".theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("null Object")
	{
		Expression<int> testExpression(&context, "nestedSelfObject.nestedSelfObject.theInt");
		doChecks(0, false, false, false, testExpression, context);
	}
}