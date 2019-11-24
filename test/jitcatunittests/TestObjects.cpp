/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "TestObjects.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

#include <iostream>

using namespace jitcat;
using namespace jitcat::Reflection;
using namespace TestObjects;

NestedReflectedObject::NestedReflectedObject():
	someString("test"),
	someInt(21),
	someFloat(1.1f),
	someBoolean(true),
	nullObject(nullptr),
	nullCircularRefObject(nullptr)
{
}


void NestedReflectedObject::reflect(ReflectedTypeInfo& typeInfo)
{
	typeInfo
		.addMember("someString", &NestedReflectedObject::someString)
		.addMember("someInt", &NestedReflectedObject::someInt)
		.addMember("someFloat", &NestedReflectedObject::someFloat)
		.addMember("someBoolean", &NestedReflectedObject::someBoolean)
		.addMember("nullObject", &NestedReflectedObject::nullObject)
		.addMember("nullCircularRefObject", &NestedReflectedObject::nullCircularRefObject, MF::isWritable)
		.addMember("emptyCircularRefList", &NestedReflectedObject::emptyCircularRefList);
}


const char* NestedReflectedObject::getTypeName()
{
	return "NestedReflectedObject";
}


ReflectedObject::ReflectedObject():
	nestedSelfObject(nullptr),
	nestedObjectPointer(nullptr),
	nestedObjectUniquePointer(nullptr),
	nullObject(nullptr),
	text("Hello!"),
	numberString("123.4"),
	theInt(42),
	largeInt(1234567),
	aFloat(999.9f),
	smallFloat(0.5f),
	zeroFloat(0.0f),
	aBoolean(true),
	no(false)

{
}


TestObjects::ReflectedObject::~ReflectedObject()
{
	delete nestedSelfObject;
	delete nestedObjectPointer;
}


void ReflectedObject::createNestedObjects()
{
	floatVector.push_back(33.3f);
	floatVector.push_back(123.5f);

	intToStringMap[1] = "four";
	intToStringMap[2] = "six";

	nestedSelfObject = new ReflectedObject();
	nestedObjectPointer = new NestedReflectedObject();
	nestedObjectUniquePointer.reset(new NestedReflectedObject());

	objectVector.emplace_back();

	reflectableObjectsVector.push_back(nestedObjectPointer);
	reflectableObjectsVector.push_back(nestedObjectUniquePointer.get());
	reflectableUniqueObjectsVector.emplace_back(new NestedReflectedObject());
	reflectableUniqueObjectsVector.emplace_back(new NestedReflectedObject());

	reflectableObjectsMapCustomCompare["one"] = nestedObjectPointer;
	reflectableObjectsMapCustomCompare["two"] = nestedObjectUniquePointer.get();

	reflectableObjectsMap["one"] = nestedObjectPointer;
	reflectableObjectsMap["two"] = nestedObjectUniquePointer.get();
	reflectableUniqueObjectsMap.emplace("one", new NestedReflectedObject());
	reflectableUniqueObjectsMap.emplace("two", new NestedReflectedObject());
}


void ReflectedObject::createNullObjects()
{
	reflectableObjectsVector.push_back(nullptr);
	reflectableObjectsVector.push_back(nullptr);
	reflectableObjectsMap["one"] = nullptr;
	reflectableObjectsMap["two"] = nullptr;
}


void ReflectedObject::reflect(ReflectedTypeInfo& typeInfo)
{
	typeInfo
		.addMember("getFloat", &ReflectedObject::getFloat)
		.addMember("getInt", &ReflectedObject::getInt)
		.addMember("getBoolean", &ReflectedObject::getBoolean)
		.addMember("getString", &ReflectedObject::getString)
		//.addMember("getStringRef", &ReflectedObject::getStringRef) Not yet supported
		.addMember("getObject", &ReflectedObject::getObject)
		.addMember("getObject2", &ReflectedObject::getObject2)
		
		.addMember("doSomething", &ReflectedObject::doSomething)

		.addMember("getConstantFloat", &ReflectedObject::getConstantFloat)
		.addMember("getConstInt", &ReflectedObject::getConstInt)
		.addMember("getConstBool", &ReflectedObject::getConstBool)
		.addMember("getConstString", &ReflectedObject::getConstString)
		.addMember("getConstObject", &ReflectedObject::getConstObject)
		.addMember("doSomethingConst", &ReflectedObject::doSomethingConst)

		.addMember("checkTheseValues", &ReflectedObject::checkTheseValues)
		.addMember("returnThisString", &ReflectedObject::returnThisString)
		.addMember("addToString", &ReflectedObject::addToString)
		.addMember("getThisObject", &ReflectedObject::getThisObject)

		.addMember("numberString", &ReflectedObject::numberString)
		.addMember("text", &ReflectedObject::text, MF::isWritable)
		.addMember("theInt", &ReflectedObject::theInt, MF::isWritable)
		.addMember("largeInt", &ReflectedObject::largeInt)
		.addMember("aFloat", &ReflectedObject::aFloat, MF::isWritable)
		.addMember("smallFloat", &ReflectedObject::smallFloat, MF::isWritable)
		.addMember("zeroFloat", &ReflectedObject::zeroFloat)
		.addMember("aBoolean", &ReflectedObject::aBoolean, MF::isWritable)
		.addMember("no", &ReflectedObject::no)

		
		.addMember("nestedSelfObject", &ReflectedObject::nestedSelfObject, MF::isWritable)
		.addMember("nullObject", &ReflectedObject::nullObject)
		.addMember("nestedObject", &ReflectedObject::nestedObject)
		.addMember("nestedObjectPointer", &ReflectedObject::nestedObjectPointer, MF::isWritable)
		.addMember("nestedObjectUniquePointer", &ReflectedObject::nestedObjectUniquePointer)
		
		.addMember("objectVector", &ReflectedObject::objectVector)
		.addMember("reflectableObjectsVector", &ReflectedObject::reflectableObjectsVector)
		.addMember("reflectableUniqueObjectsVector", &ReflectedObject::reflectableUniqueObjectsVector)

		.addMember("floatVector", &ReflectedObject::floatVector)

		.addMember("intToStringMap", &ReflectedObject::intToStringMap)

		.addMember("reflectableObjectsMap", &ReflectedObject::reflectableObjectsMap)
		.addMember("reflectableObjectsMapCustomCompare", &ReflectedObject::reflectableObjectsMapCustomCompare)
		.addMember("reflectableUniqueObjectsMap", &ReflectedObject::reflectableUniqueObjectsMap);
	
}


const char* ReflectedObject::getTypeName()
{
	return "ReflectedObject";
}


float ReflectedObject::getFloat()
{
	return aFloat;
}


int ReflectedObject::getInt()
{
	return theInt;
}


bool ReflectedObject::getBoolean()
{
	return aBoolean;
}


std::string ReflectedObject::getString()
{
	return text;
}


const std::string& TestObjects::ReflectedObject::getStringRef()
{
	return text;
}


ReflectedObject* ReflectedObject::getObject()
{
	return nestedSelfObject;
}


ReflectedObject* TestObjects::ReflectedObject::getObject2(const std::string& name, bool amITrue)
{
	return nestedSelfObject;
}


void ReflectedObject::doSomething()
{
	std::cout << "TEST DoingSomething\n";
}


float ReflectedObject::getConstantFloat() const
{
	return aFloat;
}


int ReflectedObject::getConstInt() const
{
	return theInt;
}


bool ReflectedObject::getConstBool() const
{
	return aBoolean;
}


std::string ReflectedObject::getConstString() const
{
	return text;
}


ReflectedObject* ReflectedObject::getConstObject() const
{
	return nestedSelfObject;
}


void ReflectedObject::doSomethingConst() const
{
	std::cout << "TEST DoingSomethingConst\n";
}


void ReflectedObject::checkTheseValues(bool amITrue, int someAmount, const std::string& someText, ReflectedObject* someObject)
{
	std::cout << "TEST CheckingValues " << amITrue << " " << someAmount << " " << someText  << " " << someObject << "\n";
}


std::string ReflectedObject::returnThisString(const std::string& aString) const
{
	return aString;
}


std::string ReflectedObject::addToString(const std::string& text, float number)
{
	return Tools::append(text, number);
}


ReflectedObject* ReflectedObject::getThisObject(ReflectedObject* someObject) const
{
	return someObject;
}
