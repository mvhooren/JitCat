/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/TypeInfo.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


TEST_CASE("Custom Types", "[customtypes]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	std::unique_ptr<ReflectedObject> objectUniquePtr(new ReflectedObject());
	ExpressionErrorManager errorManager;
	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();

	const char* customTypeName2 = "MyType2";
	TypeRegistry::get()->removeType(customTypeName2);
	CustomTypeInfo* customType = new CustomTypeInfo(customTypeName2);
	TypeRegistry::get()->registerType(customTypeName2, customType);
	customType->addFloatMember("myFloat", 0.001f);
	customType->addIntMember("myInt", 54321);
	customType->addStringMember("myString", "foo");
	customType->addBoolMember("myBoolean", true);
	customType->addObjectMember("myObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject2", objectUniquePtr.get(), objectTypeInfo);
	ReflectableInstance typeInstance(customType->construct(), customType);

	const char* customTypeName3 = "MyType3";
	TypeRegistry::get()->removeType(customTypeName3);
	CustomTypeInfo* customType2 = new CustomTypeInfo(customTypeName3);
	TypeRegistry::get()->registerType(customTypeName3, customType2);
	customType2->addFloatMember("myNullFloat", 0.001f);
	customType2->addIntMember("myNullInt", 54321);
	customType2->addStringMember("myNullString", "foo");
	customType2->addBoolMember("myNullBoolean", true);
	customType2->addObjectMember("myNullObject3", &reflectedObject, objectTypeInfo);

	//The case where the pointer is set to null manually
	std::any instanceAny((Reflectable*)typeInstance.getReflectable());
	std::any nullAny((Reflectable*)nullptr);
	static_cast<CustomTypeObjectMemberInfo*>(customType->getMemberInfo("myNullObject"))->assign(instanceAny, nullAny);
	//The case where the reflectable handle is set to null through deletion of the observed object.
	objectUniquePtr.reset(nullptr);

	CatRuntimeContext context("customTypes", &errorManager);
	context.addScope(&reflectedObject, true);
	context.addScope(customType2, nullptr, false);
	context.addScope(customType, typeInstance.getReflectable(), false);

	SECTION("Float Variable")
	{
		Expression<float> testExpression(&context, "myFloat");
		doChecks(0.001f, false, false, false, testExpression, context);
	}
	SECTION("Int Variable")
	{
		Expression<int> testExpression(&context, "myInt");
		doChecks(54321, false, false, false, testExpression, context);
	}
	SECTION("String Variable")
	{
		Expression<std::string> testExpression(&context, "myString");
		doChecks(std::string("foo"), false, false, false, testExpression, context);
	}
	SECTION("Boolean Variable")
	{
		Expression<bool> testExpression(&context, "myBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Object Variable")
	{
		Expression<ReflectedObject*> testExpression(&context, "myObject");
		doChecks(&reflectedObject, false, false, false, testExpression, context);
	}
	SECTION("Null object Variable")
	{
		Expression<ReflectedObject*> testExpression(&context, "myNullObject.nullObject");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null object Variable")
	{
		Expression<ReflectedObject*> testExpression(&context, "myNullObject2.nullObject");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}

	SECTION("Null base float Variable")
	{
		Expression<float> testExpression(&context, "myNullFloat");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Null base int Variable")
	{
		Expression<int> testExpression(&context, "myNullInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null base string Variable")
	{
		Expression<std::string> testExpression(&context, "myNullString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Null base boolean Variable")
	{
		Expression<bool> testExpression(&context, "myNullBoolean");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Null base object Variable")
	{
		Expression<ReflectedObject*> testExpression(&context, "myNullObject3");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}

	SECTION("Added Float Variable")
	{
		customType->addFloatMember("anotherFloat", 1.234f);
		Expression<float> testExpression(&context, "anotherFloat");
		doChecks(1.234f, false, false, false, testExpression, context);
	}
	SECTION("Added Int Variable")
	{
		customType->addIntMember("anotherInt", 12345);
		Expression<int> testExpression(&context, "anotherInt");
		doChecks(12345, false, false, false, testExpression, context);
	}
	SECTION("Added String Variable")
	{
		customType->addStringMember("anotherString", "bar");
		Expression<std::string> testExpression(&context, "anotherString");
		doChecks(std::string("bar"), false, false, false, testExpression, context);
	}
	SECTION("Added Boolean Variable")
	{
		customType->addBoolMember("anotherBoolean", false);
		Expression<bool> testExpression(&context, "anotherBoolean");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Added Object Variable")
	{
		customType->addObjectMember("anotherObject", reflectedObject.nestedSelfObject, objectTypeInfo);
		Expression<ReflectedObject*> testExpression(&context, "anotherObject");
		doChecks(reflectedObject.nestedSelfObject, false, false, false, testExpression, context);
	}

	TypeInfo::destroy(customType);
	TypeInfo::destroy(customType2);
}