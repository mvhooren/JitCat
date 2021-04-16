/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/StringConstantPool.h"
#include "jitcat/TypeInfo.h"
#include "PrecompilationTest.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


TEST_CASE("ExpressionAny", "[ExpressionAny]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	CatGenericType genericType = CatGenericType(objectTypeInfo);
	CatRuntimeContext context("expressionAny", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addScope(&reflectedObject, true);	

	SECTION("Literal Float")
	{
		ExpressionAny testExpression(&context, "1.1f");
		doChecks(1.1f, false, true, true, testExpression, context);
	}
	SECTION("Literal Int")
	{
		ExpressionAny testExpression(&context, "11");
		doChecks(11, false, true, true, testExpression, context);
	}
	SECTION("Literal Boolean true")
	{
		ExpressionAny testExpression(&context, "true");
		doChecks(true, false, true, true, testExpression, context);
	}
	SECTION("Literal Boolean false")
	{
		ExpressionAny testExpression(&context, "false");
		doChecks(false, false, true, true, testExpression, context);
	}
	SECTION("Literal String")
	{
		ExpressionAny testExpression(&context, "\"test\"");
		doChecks(AST::StringConstantPool::getString("test"), false, true, true, testExpression, context);
	}
	SECTION("Float Variable")
	{
		ExpressionAny testExpression(&context, "aFloat");
		doChecks(999.9f, false, false, false, testExpression, context);
	}
	SECTION("Double Variable")
	{
		ExpressionAny testExpression(&context, "aDouble");
		doChecks(reflectedObject.aDouble, false, false, false, testExpression, context);
	}
	SECTION("Int Variable")
	{
		ExpressionAny testExpression(&context, "theInt");
		doChecks(42, false, false, false, testExpression, context);
	}
	SECTION("Boolean Variable true")
	{
		ExpressionAny testExpression(&context, "aBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Boolean Variable false")
	{
		ExpressionAny testExpression(&context, "no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("String Variable")
	{
		ExpressionAny testExpression(&context, "text");
		doChecks(&reflectedObject.text, false, false, false, testExpression, context);
	}
	SECTION("Object Variable")
	{
		ExpressionAny testExpression(&context, "nestedSelfObject");
		doChecks(reflectedObject.nestedSelfObject, false, false, false, testExpression, context);
	}
	SECTION("Vector Variable")
	{
		ExpressionAny testExpression(&context, "reflectableObjectsVector");
		doChecks(&reflectedObject.reflectableObjectsVector, false, false, false, testExpression, context);
	}
	SECTION("Map Variable")
	{
		ExpressionAny testExpression(&context, "reflectableObjectsMap");
		doChecks(&reflectedObject.reflectableObjectsMap, false, false, false, testExpression, context);
	}
	SECTION("Void")
	{
		ExpressionAny testExpression(&context, "doSomething()");
		doCommonChecks(&testExpression, false, false, false, context);
		REQUIRE(testExpression.getType().isVoidType());
		std::any value = testExpression.getValue(&context);
		CHECK_FALSE(value.has_value());
		std::any value2 = testExpression.getInterpretedValue(&context);
		CHECK_FALSE(value2.has_value());
	}
}