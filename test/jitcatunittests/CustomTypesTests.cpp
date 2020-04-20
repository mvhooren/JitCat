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
#include "jitcat/TypeInfoDeleter.h"
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
	std::unique_ptr<ReflectedObject> objectUniquePtr(std::make_unique<ReflectedObject>());
	ExpressionErrorManager errorManager;
	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	TypeInfo* vectorTypeInfo = TypeRegistry::get()->registerType<TestVector4>();

	const char* customTypeName2 = "MyType2";
	TypeRegistry::get()->removeType(customTypeName2);
	std::unique_ptr<CustomTypeInfo, TypeInfoDeleter> customType = makeTypeInfo<CustomTypeInfo>(customTypeName2);
	TypeRegistry::get()->registerType(customTypeName2, customType.get());
	customType->addFloatMember("myFloat", 0.001f);
	customType->addIntMember("myInt", 54321);
	customType->addStringMember("myString", "foo");
	customType->addBoolMember("myBoolean", true);
	customType->addObjectMember("myObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject2", objectUniquePtr.get(), objectTypeInfo);
	customType->addDataObjectMember("myVector4DataObject", vectorTypeInfo);
	ObjectInstance typeInstance(customType->construct(), customType.get());
	std::any vector4 = customType->getMemberInfo("myVector4DataObject")->getMemberReference(typeInstance.getObject());
	TestVector4* vector4Member = std::any_cast<TestVector4*>(vector4);
	vector4Member->x = 42.0f;
	const char* customTypeName3 = "MyType3";
	TypeRegistry::get()->removeType(customTypeName3);
	std::unique_ptr<CustomTypeInfo, TypeInfoDeleter> customType2 = makeTypeInfo<CustomTypeInfo>(customTypeName3);
	TypeRegistry::get()->registerType(customTypeName3, customType2.get());
	customType2->addFloatMember("myNullFloat", 0.001f);
	customType2->addIntMember("myNullInt", 54321);
	customType2->addStringMember("myNullString", "foo");
	customType2->addBoolMember("myNullBoolean", true);
	customType2->addObjectMember("myNullObject3", &reflectedObject, objectTypeInfo);
	customType2->addConstant("myIntConstant", 3);
	customType2->addConstant("myFloatConstant", 3.141592f);
	customType2->addConstant("myBoolConstant", false);
	customType2->addConstant("myStringConstant", std::string("custom"));
	customType2->addConstant("myVectorConstant", TestVector4(1.0f, 1.1f, 1.11f, 1.111f));
	static std::unique_ptr<TestVector4> v4 = std::make_unique<TestVector4>(2.0f, 2.2f, 2.22f, 2.222f);
	customType2->addConstant("myVectorConstantPtr", v4.get());

	//The case where the pointer is set to null manually
	std::any instanceAny = typeInstance.getObjectAsAny();
	std::any nullAny = customType->getMemberInfo("myNullObject")->catType.createNullPtr();
	static_cast<CustomTypeObjectMemberInfo*>(customType->getMemberInfo("myNullObject"))->assign(instanceAny, nullAny);
	//The case where the reflectable handle is set to null through deletion of the observed object.
	objectUniquePtr.reset(nullptr);

	CatRuntimeContext context("customTypes", &errorManager);
	context.addScope(&reflectedObject, true);
	context.addScope(customType2.get(), nullptr, false);
	context.addScope(customType.get(), typeInstance.getObject(), false);

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
	SECTION("Object Ptr Variable")
	{
		Expression<ReflectedObject*> testExpression(&context, "myObject");
		doChecks(&reflectedObject, false, false, false, testExpression, context);
	}
	SECTION("Object Value Variable")
	{
		Expression<TestVector4> testExpression(&context, "myVector4DataObject");
		doChecks(*vector4Member, false, false, false, testExpression, context);
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

	SECTION("Float Static Constant")
	{
		Expression<float> testExpression(&context, "myFloatConstant");
		doChecks(3.141592f, false, true, false, testExpression, context);
	}
	SECTION("Int Static Constant")
	{
		Expression<int> testExpression(&context, "myIntConstant");
		doChecks(3, false, true, false, testExpression, context);
	}
	SECTION("String Static Constant")
	{
		Expression<std::string> testExpression(&context, "myStringConstant");
		doChecks(std::string("custom"), false, true, false, testExpression, context);
	}
	SECTION("Boolean Static Constant")
	{
		Expression<bool> testExpression(&context, "myBoolConstant");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("Object Ptr Static Constant")
	{
		Expression<TestVector4*> testExpression(&context, "myVectorConstantPtr");
		doChecks(v4.get(), false, true, false, testExpression, context);
	}
	SECTION("Object Value Static Constant")
	{
		Expression<TestVector4> testExpression(&context, "myVectorConstant");
		doChecks(TestVector4(1.0f, 1.1f, 1.11f, 1.111f), false, true, false, testExpression, context);
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
}