/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeInstance.h"
#include "jitcat/TypeInfo.h"
#include "TestHelperFunctions.h"
#include "TestObjects.h"

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


TEST_CASE("Assign tests", "[assign]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("Assign", &errorManager);
	context.addScope(&reflectedObject, true);

	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	const char* customTypeName = "MyType";
	TypeRegistry::get()->removeType(customTypeName);
	std::unique_ptr<CustomTypeInfo> customType(new CustomTypeInfo(customTypeName));
	TypeRegistry::get()->registerType(customTypeName, customType.get());
	customType->addFloatMember("myFloat", 0.001f);
	customType->addIntMember("myInt", 54321);
	customType->addStringMember("myString", "foo");
	customType->addBoolMember("myBoolean", true);
	customType->addObjectMember("myObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject", &reflectedObject, objectTypeInfo);
	std::unique_ptr<CustomTypeInstance> typeInstance(customType->createInstance());
	context.addCustomTypeScope(customType.get(), typeInstance.get());

	const char* customStaticTypeName = "MyStaticType";
	TypeRegistry::get()->removeType(customStaticTypeName);
	std::unique_ptr<CustomTypeInfo> customStaticType(new CustomTypeInfo(customStaticTypeName));
	customStaticType->addObjectMember("myStaticObject", &reflectedObject, objectTypeInfo);
	customStaticType->addObjectMember("myStaticCustomObject", typeInstance.get(), customType.get());
	std::unique_ptr<CustomTypeInstance> staticTypeInstance(customStaticType->createInstance());
	context.addCustomTypeScope(customStaticType.get(), staticTypeInstance.get(), true);

	SECTION("Assign reflected int")
	{
		Expression<void> testExpression(&context, "theInt = -99");
		checkAssignment(reflectedObject.theInt, -99, false, false, false, testExpression, context);
	}
	SECTION("Assign reflected float")
	{
		Expression<void> testExpression(&context, "aFloat = 11.0f");
		checkAssignment(reflectedObject.aFloat, 11.0f, false, false, false, testExpression, context);
	}
	SECTION("Assign reflected bool")
	{
		Expression<void> testExpression(&context, "aBoolean = no");
		checkAssignment(reflectedObject.aBoolean, false, false, false, false, testExpression, context);
	}
	SECTION("Assign reflected string")
	{
		Expression<void> testExpression(&context, "text = \"World!\"");
		checkAssignment(reflectedObject.text, std::string("World!"), false, false, false, testExpression, context);
	}
	SECTION("Assign reflected object")
	{
		Expression<void> testExpression(&context, "nestedObjectPointer = nestedObject");
		checkAssignment((Reflectable*&)reflectedObject.nestedObjectPointer, (Reflectable*)&reflectedObject.nestedObject, false, false, false, testExpression, context);	
	}
	SECTION("Assign reflected object 2")
	{
		Expression<void> testExpression(&context, "nestedSelfObject.nestedObjectPointer = nestedObject");
		checkAssignment((Reflectable * &)reflectedObject.nestedSelfObject->nestedObjectPointer, (Reflectable*)& reflectedObject.nestedObject, false, false, false, testExpression, context);
	}
	SECTION("Assign custom object")
	{
		Expression<void> testExpression(&context, "myNullObject = nestedSelfObject");
		doCommonChecks(&testExpression, false, false, false, context);
		testExpression.getValue(&context);
		testExpression.getInterpretedValue(&context);
	}
	SECTION("Assign static custom object")
	{
		Expression<void> testExpression(&context, "myStaticCustomObject.myNullObject = nestedSelfObject");
		doCommonChecks(&testExpression, false, false, false, context);
		testExpression.getValue(&context);
		testExpression.getInterpretedValue(&context);
	}
	SECTION("Assign nonWritable int")
	{
		Expression<void> testExpression(&context, "largeInt = -99");
		checkAssignment(reflectedObject.theInt, -99, true, false, false, testExpression, context);
	}
	SECTION("Assign nonWritable reflected float")
	{
		Expression<void> testExpression(&context, "zeroFloat = 11.0f");
		checkAssignment(reflectedObject.zeroFloat, 11.0f, true, false, false, testExpression, context);	
	}
	SECTION("Assign nonWritable reflected bool")
	{
		Expression<void> testExpression(&context, "no = true");
		checkAssignment(reflectedObject.no, true, true, false, false, testExpression, context);		
	}
	SECTION("Assign nonWritable reflected string")
	{
		Expression<void> testExpression(&context, "numberString = \"456.7\"");
		checkAssignment(reflectedObject.numberString, std::string("456.7"), true, false, false, testExpression, context);		
	}
	SECTION("Assign nonWritable reflected object")
	{
		Expression<void> testExpression(&context, "nestedObjectUniquePointer = nestedObject");
		checkAssignment((Reflectable*&)reflectedObject.nestedObjectPointer, (Reflectable*)&reflectedObject.nestedObject, true, false, false, testExpression, context);	
	}

	SECTION("Assign custom int")
	{
		Expression<void> testExpression(&context, "myInt = -99");
		checkAssignmentCustom(typeInstance.get(), "myInt", -99, false, false, false, testExpression, context);
	}
	SECTION("Assign custom float")
	{
		Expression<void> testExpression(&context, "myFloat = 11.0f");
		checkAssignmentCustom(typeInstance.get(), "myFloat", 11.0f, false, false, false, testExpression, context);	
	}
	SECTION("Assign custom bool")
	{
		Expression<void> testExpression(&context, "myBoolean = false");
		checkAssignmentCustom(typeInstance.get(), "myBoolean", false, false, false, false, testExpression, context);		
	}
	SECTION("Assign custom string")
	{
		Expression<void> testExpression(&context, "myString = \"bar\"");
		checkAssignmentCustom(typeInstance.get(), "myString", std::string("bar"), false, false, false, testExpression, context);		
	}
	SECTION("Assign custom object")
	{
		Expression<void> testExpression(&context, "myObject = nestedSelfObject");
		checkAssignmentCustom(typeInstance.get(), "myObject", (Reflectable*)reflectedObject.nestedSelfObject, false, false, false, testExpression, context);		
	}
}


TEST_CASE("Expression assign tests", "[assign][expressionassign]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("Assign", &errorManager);
	context.addScope(&reflectedObject, true);

	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	const char* customTypeName = "MyType";
	TypeRegistry::get()->removeType(customTypeName);
	std::unique_ptr<CustomTypeInfo> customType(new CustomTypeInfo(customTypeName));
	TypeRegistry::get()->registerType(customTypeName, customType.get());
	customType->addFloatMember("myFloat", 0.001f);
	customType->addIntMember("myInt", 54321);
	customType->addStringMember("myString", "foo");
	customType->addBoolMember("myBoolean", true);
	customType->addObjectMember("myObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject", &reflectedObject, objectTypeInfo);
	std::unique_ptr<CustomTypeInstance> typeInstance(customType->createInstance());
	context.addCustomTypeScope(customType.get(), typeInstance.get());


	SECTION("Assign reflected int")
	{
		ExpressionAssignment<int> testExpression(&context, "theInt");
		checkAssignExpression(reflectedObject.theInt, -99, false, testExpression, context);
	}
	SECTION("Assign reflected float")
	{
		ExpressionAssignment<float> testExpression(&context, "aFloat");
		checkAssignExpression(reflectedObject.aFloat, 11.0f, false, testExpression, context);
	}
	SECTION("Assign reflected bool")
	{
		ExpressionAssignment<bool> testExpression(&context, "aBoolean");
		checkAssignExpression(reflectedObject.aBoolean, false, false, testExpression, context);
	}
	SECTION("Assign reflected string")
	{
		ExpressionAssignment<std::string> testExpression(&context, "text");
		checkAssignExpression(reflectedObject.text, std::string("World!"), false, testExpression, context);
	}
	SECTION("Assign reflected object")
	{
		ExpressionAssignment<NestedReflectedObject*> testExpression(&context, "nestedObjectPointer");
		checkAssignExpression(reflectedObject.nestedObjectPointer, &reflectedObject.nestedObject, false, testExpression, context);	
	}

	SECTION("Assign nonWritable int")
	{
		ExpressionAssignment<int> testExpression(&context, "largeInt");
		checkAssignExpression(reflectedObject.theInt, -99, true, testExpression, context);
	}
	SECTION("Assign nonWritable reflected float")
	{
		ExpressionAssignment<float> testExpression(&context, "zeroFloat");
		checkAssignExpression(reflectedObject.zeroFloat, 11.0f, true, testExpression, context);	
	}
	SECTION("Assign nonWritable reflected bool")
	{
		ExpressionAssignment<bool> testExpression(&context, "no");
		checkAssignExpression(reflectedObject.no, true, true, testExpression, context);		
	}
	SECTION("Assign nonWritable reflected string")
	{
		ExpressionAssignment<std::string> testExpression(&context, "numberString");
		checkAssignExpression(reflectedObject.numberString, std::string("456.7"), true, testExpression, context);		
	}
	SECTION("Assign nonWritable reflected object")
	{
		ExpressionAssignment<NestedReflectedObject*> testExpression(&context, "nestedObjectUniquePointer");
		checkAssignExpression(reflectedObject.nestedObjectPointer, &reflectedObject.nestedObject, true, testExpression, context);	
	}

	SECTION("Assign custom int")
	{
		ExpressionAssignment<int> testExpression(&context, "myInt");
		checkAssignExpressionCustom(typeInstance.get(), "myInt", -99, false, testExpression, context);
	}
	SECTION("Assign custom float")
	{
		ExpressionAssignment<float> testExpression(&context, "myFloat");
		checkAssignExpressionCustom(typeInstance.get(), "myFloat", 11.0f, false, testExpression, context);	
	}
	SECTION("Assign custom bool")
	{
		ExpressionAssignment<bool> testExpression(&context, "myBoolean");
		checkAssignExpressionCustom(typeInstance.get(), "myBoolean", false, false, testExpression, context);		
	}
	SECTION("Assign custom string")
	{
		ExpressionAssignment<std::string> testExpression(&context, "myString");
		checkAssignExpressionCustom(typeInstance.get(), "myString", std::string("bar"), false, testExpression, context);		
	}
	SECTION("Assign custom object")
	{
		ExpressionAssignment<ReflectedObject*> testExpression(&context, "myObject");
		checkAssignExpressionCustom(typeInstance.get(), "myObject", reflectedObject.nestedSelfObject, false, testExpression, context);		
	}
}

TEST_CASE("Expression any assign tests", "[assign][expressionassign]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("Assign", &errorManager);
	context.addScope(&reflectedObject, true);

	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	const char* customTypeName = "MyType";
	TypeRegistry::get()->removeType(customTypeName);
	std::unique_ptr<CustomTypeInfo> customType(new CustomTypeInfo(customTypeName));
	TypeRegistry::get()->registerType(customTypeName, customType.get());
	customType->addFloatMember("myFloat", 0.001f);
	customType->addIntMember("myInt", 54321);
	customType->addStringMember("myString", "foo");
	customType->addBoolMember("myBoolean", true);
	customType->addObjectMember("myObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject", &reflectedObject, objectTypeInfo);
	std::unique_ptr<CustomTypeInstance> typeInstance(customType->createInstance());
	context.addCustomTypeScope(customType.get(), typeInstance.get());


	SECTION("Assign reflected int")
	{
		ExpressionAssignAny testExpression(&context, "theInt");
		checkAnyAssignExpression(reflectedObject.theInt, -99, false, testExpression, context);
	}
	SECTION("Assign reflected float")
	{
		ExpressionAssignAny testExpression(&context, "aFloat");
		checkAnyAssignExpression(reflectedObject.aFloat, 11.0f, false, testExpression, context);
	}
	SECTION("Assign reflected bool")
	{
		ExpressionAssignAny testExpression(&context, "aBoolean");
		checkAnyAssignExpression(reflectedObject.aBoolean, false, false, testExpression, context);
	}
	SECTION("Assign reflected string")
	{
		ExpressionAssignAny testExpression(&context, "text");
		checkAnyAssignExpression(reflectedObject.text, std::string("World!"), false, testExpression, context);
	}
	SECTION("Assign reflected object")
	{
		ExpressionAssignAny testExpression(&context, "nestedObjectPointer");
		checkAnyAssignExpression(reflectedObject.nestedObjectPointer, &reflectedObject.nestedObject, false, testExpression, context);
	}

	SECTION("Assign nonWritable int")
	{
		ExpressionAssignAny testExpression(&context, "largeInt");
		checkAnyAssignExpression(reflectedObject.theInt, -99, true, testExpression, context);
	}
	SECTION("Assign nonWritable reflected float")
	{
		ExpressionAssignAny testExpression(&context, "zeroFloat");
		checkAnyAssignExpression(reflectedObject.zeroFloat, 11.0f, true, testExpression, context);
	}
	SECTION("Assign nonWritable reflected bool")
	{
		ExpressionAssignAny testExpression(&context, "no");
		checkAnyAssignExpression(reflectedObject.no, true, true, testExpression, context);
	}
	SECTION("Assign nonWritable reflected string")
	{
		ExpressionAssignAny testExpression(&context, "numberString");
		checkAnyAssignExpression(reflectedObject.numberString, std::string("456.7"), true, testExpression, context);
	}
	SECTION("Assign nonWritable reflected object")
	{
		ExpressionAssignAny testExpression(&context, "nestedObjectUniquePointer");
		checkAnyAssignExpression(reflectedObject.nestedObjectPointer, &reflectedObject.nestedObject, true, testExpression, context);
	}

	SECTION("Assign custom int")
	{
		ExpressionAssignAny testExpression(&context, "myInt");
		checkAnyAssignExpressionCustom(typeInstance.get(), "myInt", -99, false, testExpression, context);
	}
	SECTION("Assign custom float")
	{
		ExpressionAssignAny testExpression(&context, "myFloat");
		checkAnyAssignExpressionCustom(typeInstance.get(), "myFloat", 11.0f, false, testExpression, context);
	}
	SECTION("Assign custom bool")
	{
		ExpressionAssignAny testExpression(&context, "myBoolean");
		checkAnyAssignExpressionCustom(typeInstance.get(), "myBoolean", false, false, testExpression, context);
	}
	SECTION("Assign custom string")
	{
		ExpressionAssignAny testExpression(&context, "myString");
		checkAnyAssignExpressionCustom(typeInstance.get(), "myString", std::string("bar"), false, testExpression, context);
	}
	SECTION("Assign custom object")
	{
		ExpressionAssignAny testExpression(&context, "myObject");
		checkAnyAssignExpressionCustom(typeInstance.get(), "myObject", reflectedObject.nestedSelfObject, false, testExpression, context);
	}
}