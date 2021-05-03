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


TEST_CASE("Containers tests: Vector", "[containers][vector]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("vectorContainer", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "vectorStaticScope");

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
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "uniqueVectorStaticScope");

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
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "mapStaticScope");	

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
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "customMapStaticScope");

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
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "uniqueMapStaticScope");

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
		Expression<int> testExpression(&context, "reflectableUniqueObjectsMap.size()");
		doChecks((int)reflectedObject.reflectableUniqueObjectsMap.size(), false, false, false, testExpression, context);
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


TEST_CASE("Containers tests: Unordered_Map", "[containers][unordered_map]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("unordered_map_container", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "unordered_mapStaticScope");	

	SECTION("Unordered_Map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsUnorderedMap[42]");
		doChecks(reflectedObject.reflectableObjectsUnorderedMap[42], false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map get float")
	{
		Expression<float> testExpression(&context, "stringToFloatUnorderedMap[\"fortytwo\"]");
		doChecks(reflectedObject.stringToFloatUnorderedMap["fortytwo"], false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map get object, invalid index")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsUnorderedMap[\"invalid\"]");
		doChecks((NestedReflectedObject*)nullptr, true, false, false, testExpression, context);
	}
	SECTION("Unordered_Map get size")
	{
		Expression<int> testExpression(&context, "reflectableObjectsUnorderedMap.size()");
		doChecks((int)reflectedObject.reflectableObjectsUnorderedMap.size(), false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map get bool")
	{
		Expression<bool> testExpression(&context, "reflectableObjectsToBoolUnorderedMap[nestedObjectPointer]");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsUnorderedMap.index(5000)");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map get object, string index not found")
	{
		Expression<float> testExpression(&context, "stringToFloatUnorderedMap[\"OneHundred\"]");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsUnorderedMap.index(-1)");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsUnorderedMap.index(0).someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map get int object")
	{
		Expression<int> testExpression(&context, "reflectableObjectsUnorderedMap.index(0).someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableObjectsUnorderedMap.index(0).someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsUnorderedMap.index(10).someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsUnorderedMap[theInt + 1].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Unordered_Map out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsUnorderedMap.index(-theInt).someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsUnorderedMap.index(0)");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null map get size")
	{
		Expression<int> testExpression(&context, "nullObject.reflectableObjectsUnorderedMap.size()");
		doChecks(0, false, false, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Array", "[containers][array]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("arrayContainer", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "arrayStaticScope");

	SECTION("Array get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "objectArray[0]");
		doChecks(&reflectedObject.objectArray[0], false, false, false, testExpression, context);
	}
	SECTION("Array get size")
	{
		Expression<int> testExpression(&context, "objectArray.size()");
		doChecks((int)reflectedObject.objectArray.size(), false, false, false, testExpression, context);
	}
	SECTION("Array get float")
	{
		Expression<float> testExpression(&context, "floatArray[1]");
		doChecks(reflectedObject.floatArray[1], false, false, false, testExpression, context);
	}
	SECTION("Array get bool")
	{
		Expression<bool> testExpression(&context, "boolArray[1]");
		doChecks(reflectedObject.boolArray[1], false, false, false, testExpression, context);
	}
	SECTION("Array get float out of range")
	{
		Expression<float> testExpression(&context, "floatArray[-1]");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Array get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "objectArray[5000]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Array get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "objectArray[-1]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Array get string object")
	{
		Expression<std::string> testExpression(&context, "objectArray[0].someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Array get int object")
	{
		Expression<int> testExpression(&context, "objectArray[0].someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Array get boolean object")
	{
		Expression<bool> testExpression(&context, "objectArray[0].someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Array out of range get string")
	{
		Expression<std::string> testExpression(&context, "objectArray[10].someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Array out of range, variable index")
	{
		Expression<int> testExpression(&context, "objectArray[theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Array out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "objectArray[-theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null array get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.objectArray[0]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null array get size")
	{
		Expression<int> testExpression(&context, "nullObject.objectArray.size()");
		doChecks(0, false, false, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Deque", "[containers][deque]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("dequeContainer", &errorManager);
	context.setPrecompilationContext(Precompilation::precompContext);
	context.addStaticScope(&reflectedObject, "dequeStaticScope");

	SECTION("Deque get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "objectUniquePtrDeque[0]");
		doChecks(reflectedObject.objectUniquePtrDeque[0].get(), false, false, false, testExpression, context);
	}
	SECTION("Deque get size")
	{
		Expression<int> testExpression(&context, "objectUniquePtrDeque.size()");
		doChecks((int)reflectedObject.objectArray.size(), false, false, false, testExpression, context);
	}
	SECTION("Deque get int")
	{
		Expression<int> testExpression(&context, "intDeque[1]");
		doChecks(reflectedObject.intDeque[1], false, false, false, testExpression, context);
	}
	SECTION("Deque get bool")
	{
		Expression<bool> testExpression(&context, "boolDeque[1]");
		doChecks(reflectedObject.boolDeque[1], false, false, false, testExpression, context);
	}
	SECTION("Deque get int out of range")
	{
		Expression<int> testExpression(&context, "intDeque[-1]");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Deque get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "objectUniquePtrDeque[5000]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Deque get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "objectUniquePtrDeque[-1]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Deque get string object")
	{
		Expression<std::string> testExpression(&context, "objectUniquePtrDeque[0].someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Deque get int object")
	{
		Expression<int> testExpression(&context, "objectUniquePtrDeque[0].someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Deque get boolean object")
	{
		Expression<bool> testExpression(&context, "objectUniquePtrDeque[0].someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Deque out of range get string")
	{
		Expression<std::string> testExpression(&context, "objectUniquePtrDeque[10].someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Deque out of range, variable index")
	{
		Expression<int> testExpression(&context, "objectUniquePtrDeque[theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Deque out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "objectUniquePtrDeque[-theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null deque get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.objectUniquePtrDeque[0]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null deque get size")
	{
		Expression<int> testExpression(&context, "nullObject.objectUniquePtrDeque.size()");
		doChecks(0, false, false, false, testExpression, context);
	}
}