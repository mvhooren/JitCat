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


TEST_CASE("Static Member Variables", "[staticmembervariables]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("staticmembervariables_tests", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Static invalid")
	{
		Expression<std::string> testExpression(&context, "ReflectedObject::staticDoesNotExist");
		doChecks(ReflectedObject::staticString, true, false, false, testExpression, context);
	}
	SECTION("Static float")
	{
		Expression<float> testExpression(&context, "ReflectedObject::staticFloat");
		doChecks(ReflectedObject::staticFloat, false, false, false, testExpression, context);
	}
	SECTION("Static double")
	{
		Expression<double> testExpression(&context, "ReflectedObject::staticDouble");
		doChecks(ReflectedObject::staticDouble, false, false, false, testExpression, context);
	}
	SECTION("Static int")
	{
		Expression<int> testExpression(&context, "ReflectedObject::staticInt");
		doChecks(ReflectedObject::staticInt, false, false, false, testExpression, context);
	}
	SECTION("Static bool")
	{
		Expression<bool> testExpression(&context, "ReflectedObject::staticBool");
		doChecks(ReflectedObject::staticBool, false, false, false, testExpression, context);
	}
	SECTION("Static string")
	{
		Expression<std::string> testExpression(&context, "ReflectedObject::staticString");
		doChecks(ReflectedObject::staticString, false, false, false, testExpression, context);
	}
	SECTION("Static object")
	{
		Expression<NestedReflectedObject> testExpression(&context, "ReflectedObject::staticObject");
		doChecks(ReflectedObject::staticObject, false, false, false, testExpression, context);
	}
	SECTION("Static object ptr")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "ReflectedObject::staticObjectPtr");
		doChecks(ReflectedObject::staticObjectPtr, false, false, false, testExpression, context);
	}
	SECTION("Static object null ptr")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "ReflectedObject::staticObjectNullPtr");
		doChecks(ReflectedObject::staticObjectNullPtr, false, false, false, testExpression, context);
	}
	SECTION("Static object unique ptr")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "ReflectedObject::staticObjectUniquePtr");
		doChecks(ReflectedObject::staticObjectUniquePtr.get(), false, false, false, testExpression, context);
	}
	SECTION("Static object unique ptr by value conversion")
	{
		Expression<NestedReflectedObject> testExpression(&context, "ReflectedObject::staticObjectUniquePtr");
		doChecks(*ReflectedObject::staticObjectUniquePtr.get(), false, false, false, testExpression, context);
	}

	SECTION("Static const float")
	{
		Expression<float> testExpression(&context, "ReflectedObject::floatConstant");
		doChecks(3.141592f, false, true, false, testExpression, context);
	}
	SECTION("Static const double")
	{
		Expression<double> testExpression(&context, "ReflectedObject::doubleConstant");
		doChecks(3.141592, false, true, false, testExpression, context);
	}
	SECTION("Static const int")
	{
		Expression<int> testExpression(&context, "ReflectedObject::intConstant");
		doChecks(42, false, true, false, testExpression, context);
	}
	SECTION("Static const bool")
	{
		Expression<bool> testExpression(&context, "ReflectedObject::boolConstant");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Static const string")
	{
		Expression<std::string> testExpression(&context, "ReflectedObject::stringConstant");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("Static const object")
	{
		Expression<TestVector4> testExpression(&context, "ReflectedObject::vectorConstant");
		doChecks(TestVector4(2.0f, 4.0f, 6.0f, 8.0f), false, true, false, testExpression, context);
	}
	SECTION("Static const object ptr")
	{
		Expression<TestVector4*> testExpression(&context, "ReflectedObject::vectorConstantPtr");
		doChecks(ReflectedObject::testVectorConst.get(), false, true, false, testExpression, context);
	}

	SECTION("Static vector")
	{
		Expression<int> testExpression(&context, "ReflectedObject::staticVector[0]");
		doChecks(ReflectedObject::staticVector[0], false, false, false, testExpression, context);
	}
	SECTION("Static vector out of range")
	{
		Expression<int> testExpression(&context, "ReflectedObject::staticVector[10]");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Static map")
	{
		Expression<std::string> testExpression(&context, "ReflectedObject::staticMap[1.0f]");
		doChecks(ReflectedObject::staticMap[1.0f], false, false, false, testExpression, context);
	}
	SECTION("Static string")
	{
		Expression<int> testExpression(&context, "ReflectedObject::staticStringMap[\"one\"]");
		doChecks(ReflectedObject::staticStringMap["one"], false, false, false, testExpression, context);
	}
	SECTION("Static vector4")
	{
		Expression<TestVector4> testExpression(&context, "TestVector4::zero");
		doChecks(TestVector4::zero, false, false, false, testExpression, context);
	}
	SECTION("Static vector4 as pointer")
	{
		Expression<TestVector4*> testExpression(&context, "TestVector4::zero");
		doChecks(&TestVector4::zero, false, false, false, testExpression, context);
	}
}


TEST_CASE("Custom type static member Variables", "[staticmembervariables][customtypes]" ) 
{

	ExpressionErrorManager errorManager;

	const char* customTypeName2 = "MyStaticType";
	TypeRegistry::get()->removeType(customTypeName2);
	std::unique_ptr<CustomTypeInfo, TypeInfoDeleter> customType = makeTypeInfo<CustomTypeInfo>(customTypeName2);
	TypeRegistry::get()->registerType(customTypeName2, customType.get());
	
	customType->addStaticFloatMember("myStaticFloat", 0.001f);
	customType->addStaticDoubleMember("myStaticDouble", 0.002);
	customType->addStaticIntMember("myStaticInt", 42);
	customType->addStaticBoolMember("myStaticBool", true);
	customType->addStaticStringMember("myStaticString", "Hello");
	TestVector4 vector4Obj;
	TypeInfo* typeInfo = TypeRegistry::get()->registerType<TestVector4>();
	customType->addStaticObjectMember("myObjectPtr", &vector4Obj, typeInfo);
	customType->addStaticDataObjectMember("myObjectValue", typeInfo);
	ObjectInstance typeInstance(customType->construct(), customType.get());

	
	CatRuntimeContext context("customtypestaticmembervariables_tests", &errorManager);
	context.addScope(customType.get(), typeInstance.getObject(), false);

	SECTION("Static float")
	{
		Expression<float> testExpression(&context, "MyStaticType::myStaticFloat");
		doChecks(0.001f, false, false, false, testExpression, context);
	}
	SECTION("Static double")
	{
		Expression<double> testExpression(&context, "MyStaticType::myStaticDouble");
		doChecks(0.002, false, false, false, testExpression, context);
	}
	SECTION("Static int")
	{
		Expression<int> testExpression(&context, "MyStaticType::myStaticInt");
		doChecks(42, false, false, false, testExpression, context);
	}
	SECTION("Static bool")
	{
		Expression<bool> testExpression(&context, "MyStaticType::myStaticBool");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Static string")
	{
		Expression<std::string> testExpression(&context, "MyStaticType::myStaticString");
		doChecks(std::string("Hello"), false, false, false, testExpression, context);
	}

	SECTION("Static object")
	{
		Expression<TestVector4> testExpression(&context, "MyStaticType::myObjectValue");
		doChecks(TestVector4(), false, false, false, testExpression, context);
	}
	SECTION("Static object ptr")
	{
		Expression<TestVector4*> testExpression(&context, "MyStaticType::myObjectPtr");
		doChecks(&vector4Obj, false, false, false, testExpression, context);
	}
}


TEST_CASE("Static member variable in scope", "[staticmembervariables]" ) 
{

	ExpressionErrorManager errorManager;

	const char* customTypeName2 = "MyStaticType";
	TypeRegistry::get()->removeType(customTypeName2);
	std::unique_ptr<CustomTypeInfo, TypeInfoDeleter> customType = makeTypeInfo<CustomTypeInfo>(customTypeName2);
	TypeRegistry::get()->registerType(customTypeName2, customType.get());
	
	customType->addStaticFloatMember("myStaticFloat", 0.001f);
	customType->addStaticDoubleMember("myStaticDouble", 0.002);
	customType->addStaticIntMember("myStaticInt", 42);
	customType->addStaticBoolMember("myStaticBool", true);
	customType->addStaticStringMember("myStaticString", "Hello");
	TestVector4 vector4Obj;
	TypeInfo* typeInfo = TypeRegistry::get()->registerType<TestVector4>();
	customType->addStaticObjectMember("myObjectPtr", &vector4Obj, typeInfo);
	customType->addStaticDataObjectMember("myObjectValue", typeInfo);
	ObjectInstance typeInstance(customType->construct(), customType.get());

	ReflectedObject reflectedObject;

	CatRuntimeContext context("customtypestaticmembervariables_tests", &errorManager);
	context.addScope(customType.get(), typeInstance.getObject(), false);
	context.addScope(&reflectedObject, true);

	SECTION("Static invalid")
	{
		Expression<std::string> testExpression(&context, "staticDoesNotExist");
		doChecks(ReflectedObject::staticString, true, false, false, testExpression, context);
	}
	SECTION("Static float")
	{
		Expression<float> testExpression(&context, "staticFloat");
		doChecks(ReflectedObject::staticFloat, false, false, false, testExpression, context);
	}
	SECTION("Static double")
	{
		Expression<double> testExpression(&context, "staticDouble");
		doChecks(ReflectedObject::staticDouble, false, false, false, testExpression, context);
	}
	SECTION("Static int")
	{
		Expression<int> testExpression(&context, "staticInt");
		doChecks(ReflectedObject::staticInt, false, false, false, testExpression, context);
	}
	SECTION("Static bool")
	{
		Expression<bool> testExpression(&context, "staticBool");
		doChecks(ReflectedObject::staticBool, false, false, false, testExpression, context);
	}
	SECTION("Static string")
	{
		Expression<std::string> testExpression(&context, "staticString");
		doChecks(ReflectedObject::staticString, false, false, false, testExpression, context);
	}

	SECTION("Static object")
	{
		Expression<NestedReflectedObject> testExpression(&context, "staticObject");
		doChecks(ReflectedObject::staticObject, false, false, false, testExpression, context);
	}
	SECTION("Static object ptr")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "staticObjectPtr");
		doChecks(ReflectedObject::staticObjectPtr, false, false, false, testExpression, context);
	}
	SECTION("Static object null ptr")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "staticObjectNullPtr");
		doChecks(ReflectedObject::staticObjectNullPtr, false, false, false, testExpression, context);
	}
	SECTION("Static object unique ptr")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "staticObjectUniquePtr");
		doChecks(ReflectedObject::staticObjectUniquePtr.get(), false, false, false, testExpression, context);
	}
	SECTION("Static object unique ptr by value conversion")
	{
		Expression<NestedReflectedObject> testExpression(&context, "staticObjectUniquePtr");
		doChecks(*ReflectedObject::staticObjectUniquePtr.get(), false, false, false, testExpression, context);
	}

	SECTION("Static const float")
	{
		Expression<float> testExpression(&context, "floatConstant");
		doChecks(3.141592f, false, true, false, testExpression, context);
	}
	SECTION("Static const double")
	{
		Expression<double> testExpression(&context, "doubleConstant");
		doChecks(3.141592, false, true, false, testExpression, context);
	}
	SECTION("Static const int")
	{
		Expression<int> testExpression(&context, "intConstant");
		doChecks(42, false, true, false, testExpression, context);
	}
	SECTION("Static const bool")
	{
		Expression<bool> testExpression(&context, "boolConstant");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Static const string")
	{
		Expression<std::string> testExpression(&context, "stringConstant");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("Static const object")
	{
		Expression<TestVector4> testExpression(&context, "vectorConstant");
		doChecks(TestVector4(2.0f, 4.0f, 6.0f, 8.0f), false, true, false, testExpression, context);
	}
	SECTION("Static const object ptr")
	{
		Expression<TestVector4*> testExpression(&context, "vectorConstantPtr");
		doChecks(ReflectedObject::testVectorConst.get(), false, true, false, testExpression, context);
	}

	SECTION("Static vector")
	{
		Expression<int> testExpression(&context, "staticVector[0]");
		doChecks(ReflectedObject::staticVector[0], false, false, false, testExpression, context);
	}
	SECTION("Static vector out of range")
	{
		Expression<int> testExpression(&context, "staticVector[10]");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Static map")
	{
		Expression<std::string> testExpression(&context, "staticMap[1.0f]");
		doChecks(ReflectedObject::staticMap[1.0f], false, false, false, testExpression, context);
	}
	SECTION("Static string")
	{
		Expression<int> testExpression(&context, "staticStringMap[\"one\"]");
		doChecks(ReflectedObject::staticStringMap["one"], false, false, false, testExpression, context);
	}

	SECTION("Static float")
	{
		Expression<float> testExpression(&context, "myStaticFloat");
		doChecks(0.001f, false, false, false, testExpression, context);
	}
	SECTION("Static double")
	{
		Expression<double> testExpression(&context, "myStaticDouble");
		doChecks(0.002, false, false, false, testExpression, context);
	}
	SECTION("Static int")
	{
		Expression<int> testExpression(&context, "myStaticInt");
		doChecks(42, false, false, false, testExpression, context);
	}
	SECTION("Static bool")
	{
		Expression<bool> testExpression(&context, "myStaticBool");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Static string")
	{
		Expression<std::string> testExpression(&context, "myStaticString");
		doChecks(std::string("Hello"), false, false, false, testExpression, context);
	}

	SECTION("Static object")
	{
		Expression<TestVector4> testExpression(&context, "myObjectValue");
		doChecks(TestVector4(), false, false, false, testExpression, context);
	}
	SECTION("Static object ptr")
	{
		Expression<TestVector4*> testExpression(&context, "myObjectPtr");
		doChecks(&vector4Obj, false, false, false, testExpression, context);
	}
}