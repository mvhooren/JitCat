/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
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


TEST_CASE("Basic memory leak test", "[memory]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("memberFunctions", &errorManager);
	context.addScope(&reflectedObject, true);	


	SECTION("Get TestVector4")
	{
		int currentInstances = TestVector4::instanceCount;
		{
			Expression<TestVector4> testExpression(&context, "getTestVector()");
			doChecks(reflectedObject.getTestVector(), false, false, false, testExpression, context);
		}
		CHECK(currentInstances == TestVector4::instanceCount);
	}
	SECTION("Add TestVector4")
	{
		int currentInstances = TestVector4::instanceCount;
		{
			Expression<TestVector4> testExpression(&context, "addVectors(getTestVector(), getTestVectorPtr())");
			doChecks(reflectedObject.addVectors(reflectedObject.getTestVector(), *reflectedObject.getTestVectorPtr()), false, false, false, testExpression, context);
		}
		CHECK(currentInstances == TestVector4::instanceCount);
	}
}