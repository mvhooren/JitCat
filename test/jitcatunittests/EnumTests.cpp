/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/TypeInfo.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


TEST_CASE("Enum Tests", "[enum]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;

	const char* customTypeName = "MyTestType";
	TypeRegistry::get()->removeType(customTypeName);
	std::unique_ptr<CustomTypeInfo, TypeInfoDeleter> customType = makeTypeInfo<CustomTypeInfo>(customTypeName);
	TypeRegistry::get()->registerType(customTypeName, customType.get());
	customType->addEnumMember("myEnumMember", TestEnum::TestValue2);
	customType->addConstant("myEnumConstant", TestEnum::TestValue3);
	ObjectInstance typeInstance(customType.get());

	CatRuntimeContext context("enumTests", &errorManager);
	context.addScope(&reflectedObject, true);	
	context.addScope(typeInstance, false);

	SECTION("Constant")
	{
		Expression<TestEnum> testExpression(&context, "TestEnum::TestValue2");
		doChecks(TestEnum::TestValue2, false, true, false, testExpression, context);
	}
	SECTION("Static Constant")
	{
		Expression<TestEnum> testExpression(&context, "enumConstant");
		doChecks(TestEnum::TestValue1, false, true, false, testExpression, context);
	}
	SECTION("Static Constant with specific scope")
	{
		Expression<TestEnum> testExpression(&context, "ReflectedObject::enumConstant");
		doChecks(TestEnum::TestValue1, false, true, false, testExpression, context);
	}
	SECTION("Custom Constant")
	{
		Expression<TestEnum> testExpression(&context, "myEnumConstant");
		doChecks(TestEnum::TestValue3, false, true, false, testExpression, context);
	}
	SECTION("Custom Constant with specific scope")
	{
		Expression<TestEnum> testExpression(&context, "MyTestType::myEnumConstant");
		doChecks(TestEnum::TestValue3, false, true, false, testExpression, context);
	}
	SECTION("Non-Constant")
	{
		Expression<TestEnum> testExpression(&context, "select(no, TestEnum::TestValue1, TestEnum::TestValue3)");
		doChecks(TestEnum::TestValue3, false, false, false, testExpression, context);
	}
	SECTION("Static Variable")
	{
		Expression<TestEnum> testExpression(&context, "staticEnum");
		doChecks(ReflectedObject::staticEnum, false, false, false, testExpression, context);
	}
	SECTION("Member Variable")
	{
		Expression<TestEnum> testExpression(&context, "someEnum");
		doChecks(reflectedObject.someEnum, false, false, false, testExpression, context);
	}
	SECTION("Custom Member Variable")
	{
		Expression<TestEnum> testExpression(&context, "myEnumMember");
		doChecks(TestEnum::TestValue2, false, false, false, testExpression, context);
	}
	SECTION("Member function")
	{
		Expression<TestEnum> testExpression(&context, "getEnum()");
		doChecks(reflectedObject.getEnum(), false, false, false, testExpression, context);
	}
	SECTION("Const member function")
	{
		Expression<TestEnum> testExpression(&context, "getConstEnum()");
		doChecks(reflectedObject.getConstEnum(), false, false, false, testExpression, context);
	}
	SECTION("Static function")
	{
		Expression<TestEnum> testExpression(&context, "getStaticEnum()");
		doChecks(ReflectedObject::getStaticEnum(), false, false, false, testExpression, context);
	}
}