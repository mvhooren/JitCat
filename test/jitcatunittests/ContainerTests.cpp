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


TEST_CASE("Containers tests: Vector", "[containers][vector]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("vectorContainer", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Vector get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsVector[0]");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
	}
	SECTION("Vector get size")
	{
		Expression<int> testExpression(&context, "reflectableObjectsVector.size()");
		doChecks((int)reflectedObject.reflectableObjectsVector.size(), false, false, false, testExpression, context);
	}
	SECTION("Vector get non-pointer object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "objectVector[0]");
		doChecks(&reflectedObject.objectVector[0], false, false, false, testExpression, context);
	}
	SECTION("Vector get non-pointer object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "objectVector[-1]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Vector get float")
	{
		Expression<float> testExpression(&context, "floatVector[0]");
		doChecks(reflectedObject.floatVector[0], false, false, false, testExpression, context);
	}
	SECTION("Vector get bool")
	{
		Expression<bool> testExpression(&context, "boolVector[1]");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Vector get float out of range")
	{
		Expression<float> testExpression(&context, "floatVector[-1]");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Vector get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsVector[5000]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Vector get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsVector[-1]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Vector get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsVector[0].someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Vector get int object")
	{
		Expression<int> testExpression(&context, "reflectableObjectsVector[0].someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Vector get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableObjectsVector[0].someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Vector out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsVector[10].someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Vector out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsVector[theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Vector out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsVector[-theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null vector get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsVector[0]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null vector get size")
	{
		Expression<int> testExpression(&context, "nullObject.reflectableObjectsVector.size()");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Bad vector name error")
	{
		Expression<int> testExpression(&context, "badVectorName[0]");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing vector name error")
	{
		Expression<int> testExpression(&context, "[0].theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Vector of unique_ptr", "[containers][vector]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("vectorContainer", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Vector get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsVector[0]");
		doChecks(reflectedObject.reflectableUniqueObjectsVector[0].get(), false, false, false, testExpression, context);
	}
	SECTION("Vector get size")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsVector.size()");
		doChecks((int)reflectedObject.reflectableUniqueObjectsVector.size(), false, false, false, testExpression, context);
	}
	SECTION("Vector get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsVector[5000]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Vector get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsVector[-1]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Vector get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableUniqueObjectsVector[0].someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Vector get int object")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsVector[0].someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Vector get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableUniqueObjectsVector[0].someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Vector out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableUniqueObjectsVector[10].someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Vector out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsVector[theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Vector out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsVector[-theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null vector get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsVector[0]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null vector get size")
	{
		Expression<int> testExpression(&context, "nullObject.reflectableUniqueObjectsVector.size()");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Bad vector name error")
	{
		Expression<int> testExpression(&context, "badVectorName[0]");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing vector name error")
	{
		Expression<int> testExpression(&context, "[0].theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Map", "[containers][map]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("mapContainer", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap.index(0)");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
	}
	SECTION("Map get float")
	{
		Expression<float> testExpression(&context, "intToFloatMap[42]");
		doChecks(42.0f, false, false, false, testExpression, context);
	}
	SECTION("Map get object, invalid index")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[0]");
		doChecks((NestedReflectedObject*)nullptr, true, false, false, testExpression, context);
	}
	SECTION("Map get size")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMap.size()");
		doChecks((int)reflectedObject.reflectableObjectsMap.size(), false, false, false, testExpression, context);
	}
	SECTION("Map get string")
	{
		Expression<std::string> testExpression(&context, "intToStringMap[1]");
		doChecks(reflectedObject.intToStringMap[1], false, false, false, testExpression, context);
	}
	SECTION("Map get string, not found")
	{
		Expression<std::string> testExpression(&context, "intToStringMap[0]");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[\"two\"]");
		doChecks(reflectedObject.nestedObjectUniquePointer.get(), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index uppercase")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[\"ONE\"]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap.index(5000)");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index not found")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[\"OneHundred\"]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap.index(-1)");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsMap.index(0).someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Map get int object")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMap.index(0).someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Map get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableObjectsMap.index(0).someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Map out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsMap.index(10).someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Map out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMap[text].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Map out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMap.index(-theInt).someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsMap.index(0)");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null map get size")
	{
		Expression<int> testExpression(&context, "nullObject.reflectableObjectsMap.size()");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null map get object 2")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsMap[\"one\"]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Bad map name error")
	{
		Expression<int> testExpression(&context, "badMapName[\"two\"]");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing map name error")
	{
		Expression<int> testExpression(&context, "[\"one\"].theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Map with custom comparator", "[containers][map]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("mapContainer", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare.index(0)");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
	}
	SECTION("Map get object, invalid index")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare[0]");
		doChecks((NestedReflectedObject*)nullptr, true, false, false, testExpression, context);
	}
	SECTION("Map get size")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMapCustomCompare.size()");
		doChecks((int)reflectedObject.reflectableObjectsMapCustomCompare.size(), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare[\"two\"]");
		doChecks(reflectedObject.nestedObjectUniquePointer.get(), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index uppercase")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare[\"ONE\"]");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare.index(5000)");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index not found")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare[\"OneHundred\"]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare.index(-1)");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsMapCustomCompare.index(0).someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Map get int object")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMapCustomCompare.index(0).someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Map get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableObjectsMapCustomCompare.index(0).someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Map out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsMapCustomCompare.index(10).someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Map out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMapCustomCompare[text].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Map out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMapCustomCompare.index(-theInt).someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsMapCustomCompare.index(0)");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null map get size")
	{
		Expression<int> testExpression(&context, "nullObject.reflectableObjectsMapCustomCompare.size()");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null map get object 2")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsMapCustomCompare[\"one\"]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Bad map name error")
	{
		Expression<int> testExpression(&context, "badMapName[\"two\"]");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing map name error")
	{
		Expression<int> testExpression(&context, "[\"one\"].theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Map of unique_ptr", "[containers][map]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("mapContainer", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap.index(0)");
		doChecks(reflectedObject.reflectableUniqueObjectsMap.begin()->second.get(), false, false, false, testExpression, context);
	}
	SECTION("Map get object, invalid index")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap[0]");
		doChecks((NestedReflectedObject*)nullptr, true, false, false, testExpression, context);
	}
	SECTION("Map get size")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMapCustomCompare.size()");
		doChecks((int)reflectedObject.reflectableObjectsMapCustomCompare.size(), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap[\"two\"]");
		doChecks(reflectedObject.reflectableUniqueObjectsMap["two"].get(), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index uppercase")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap[\"ONE\"]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap.index(5000)");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index not found")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap[\"OneHundred\"]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap.index(-1)");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableUniqueObjectsMap.index(0).someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Map get int object")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsMap.index(0).someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Map get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableUniqueObjectsMap.index(0).someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Map out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableUniqueObjectsMap.index(10).someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Map out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsMap[text].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Map out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsMap.index(-theInt).someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableUniqueObjectsMap.index(0)");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null map get size")
	{
		Expression<int> testExpression(&context, "nullObject.reflectableUniqueObjectsMap.size()");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null map get object 2")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableUniqueObjectsMap[\"one\"]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Bad map name error")
	{
		Expression<int> testExpression(&context, "badMapName[\"two\"]");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing map name error")
	{
		Expression<int> testExpression(&context, "[\"one\"].theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
}