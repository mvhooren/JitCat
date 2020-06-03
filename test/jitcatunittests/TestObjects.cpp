/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "TestObjects.h"
#include "jitcat/ReflectedEnumTypeInfo.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

#include <iostream>

using namespace jitcat;
using namespace jitcat::Reflection;
using namespace TestObjects;

bool TestOptions::enableVerboseLogging = false;

ReflectedObject::ReflectedObject():
	v1(1.0f, 2.0f, 3.0f, 4.0f),
	v2(4.0f, 3.0f, 2.0f, 1.0f),
	numberString("123.4"),
	text("Hello!"),
	theInt(42),
	zeroInt(0),
	largeInt(1234567),
	aFloat(999.9f),
	negativeFloat(-111.1f),
	smallFloat(0.5f),
	zeroFloat(0.0f),
	aDouble(999.9),
	negativeDouble(-111.1f),
	smallDouble(0.5f),
	zeroDouble(0.0f),
	aBoolean(true),
	no(false),
	someEnum(TestEnum::TestValue2),
	nestedSelfObject(nullptr),
	nullObject(nullptr),
	nestedObjectPointer(nullptr),
	nestedObjectUniquePointer(nullptr)

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

	boolVector.push_back(true);
	boolVector.push_back(false);

	intToFloatMap[1] = 1.0f;
	intToFloatMap[2] = 2.0f;
	intToFloatMap[42] = 42.0f;

	intToStringMap[1] = "four";
	intToStringMap[2] = "six";

	nestedSelfObject = new ReflectedObject();
	nestedObjectPointer = new NestedReflectedObject();
	nestedObjectUniquePointer = std::make_unique<NestedReflectedObject>();

	objectVector.emplace_back();

	reflectableObjectsVector.push_back(nestedObjectPointer);
	reflectableObjectsVector.push_back(nestedObjectUniquePointer.get());
	reflectableUniqueObjectsVector.emplace_back(new NestedReflectedObject());
	reflectableUniqueObjectsVector.emplace_back(new NestedReflectedObject());

	reflectableObjectsUnorderedMap[1] = nestedObjectPointer;
	reflectableObjectsUnorderedMap[42] = nestedObjectUniquePointer.get();
	stringToFloatUnorderedMap["two"] = 2.0f;
	stringToFloatUnorderedMap["fortytwo"] = 42;
	reflectableObjectsToBoolUnorderedMap[nestedObjectPointer] = true;
	reflectableObjectsToBoolUnorderedMap[nestedObjectUniquePointer.get()] = false;
	boolArray[0] = true;
	boolArray[1] = false;
	floatArray[0] = 11.1f;
	floatArray[1] = 42.2f;
	
	boolDeque.push_back(false);
	boolDeque.push_back(true);
	intDeque.push_back(42);
	intDeque.push_back(58);
	objectUniquePtrDeque.push_back(std::make_unique<NestedReflectedObject>());
	objectUniquePtrDeque.push_back(std::make_unique<NestedReflectedObject>());

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
		.addMember("getDouble", &ReflectedObject::getDouble)
		.addMember("getInt", &ReflectedObject::getInt)
		.addMember("getBoolean", &ReflectedObject::getBoolean)
		.addMember("getString", &ReflectedObject::getString)
		.addMember("getEnum", &ReflectedObject::getEnum)
		//.addMember("getStringRef", &ReflectedObject::getStringRef) Not yet supported
		.addMember("getObject", &ReflectedObject::getObject)
		.addMember("getObject2", &ReflectedObject::getObject2)
		.addMember("getTestVector", &ReflectedObject::getTestVector)
		.addMember("getTestVectorRef", &ReflectedObject::getTestVectorRef)
		.addMember("getTestVectorConstRef", &ReflectedObject::getTestVectorConstRef)
		.addMember("getTestVectorPtr", &ReflectedObject::getTestVectorPtr)
		.addMember("getConstTestVector", &ReflectedObject::getConstTestVector)
		.addMember("v1", &ReflectedObject::v1, MF::isWritable)
		.addMember("v2", &ReflectedObject::v2)
		.addMember("addVectors", &ReflectedObject::addVectors)

		.addMember("doSomething", &ReflectedObject::doSomething)

		.addMember("getConstantFloat", &ReflectedObject::getConstantFloat)
		.addMember("getConstantDouble", &ReflectedObject::getConstantDouble)
		.addMember("getConstInt", &ReflectedObject::getConstInt)
		.addMember("getConstBool", &ReflectedObject::getConstBool)
		.addMember("getConstString", &ReflectedObject::getConstString)
		.addMember("getConstEnum", &ReflectedObject::getConstEnum)
		.addMember("getConstObject", &ReflectedObject::getConstObject)
		.addMember("doSomethingConst", &ReflectedObject::doSomethingConst)

		.addMember("checkTheseValues", &ReflectedObject::checkTheseValues)
		.addMember("returnThisString", &ReflectedObject::returnThisString)
		.addMember("addToString", &ReflectedObject::addToString)
		.addMember("getThisObject", &ReflectedObject::getThisObject)

		.addMember("getStaticFloat", &ReflectedObject::getStaticFloat)
		.addMember("getStaticDouble", &ReflectedObject::getStaticDouble)
		.addMember("getStaticInt", &ReflectedObject::getStaticInt)
		.addMember("getStaticBool", &ReflectedObject::getStaticBool)
		.addMember("getStaticString", &ReflectedObject::getStaticString)
		.addMember("getStaticEnum", &ReflectedObject::getStaticEnum)
		.addMember("getStaticObject", &ReflectedObject::getStaticObject)
		.addMember("getStaticConstObject", &ReflectedObject::getStaticConstObject)
		.addMember("getStaticObjectRef", &ReflectedObject::getStaticObjectRef)
		.addMember("getStaticObjectConstRef", &ReflectedObject::getStaticObjectConstRef)
		.addMember("getStaticObjectPtr", &ReflectedObject::getStaticObjectPtr)
		
		.addMember("numberString", &ReflectedObject::numberString)
		.addMember("text", &ReflectedObject::text, MF::isWritable)
		.addMember("theInt", &ReflectedObject::theInt, MF::isWritable)
		.addMember("zeroInt", &ReflectedObject::zeroInt)
		.addMember("largeInt", &ReflectedObject::largeInt)
		.addMember("aFloat", &ReflectedObject::aFloat, MF::isWritable)
		.addMember("negativeFloat", &ReflectedObject::negativeFloat, MF::isWritable)
		.addMember("smallFloat", &ReflectedObject::smallFloat, MF::isWritable)
		.addMember("zeroFloat", &ReflectedObject::zeroFloat)
		.addMember("aDouble", &ReflectedObject::aDouble, MF::isWritable)
		.addMember("negativeDouble", &ReflectedObject::negativeDouble, MF::isWritable)
		.addMember("smallDouble", &ReflectedObject::smallDouble, MF::isWritable)
		.addMember("zeroDouble", &ReflectedObject::zeroDouble)
		.addMember("aBoolean", &ReflectedObject::aBoolean, MF::isWritable)
		.addMember("no", &ReflectedObject::no)
		.addMember("someEnum", &ReflectedObject::someEnum)

		.addConstant("intConstant", 42)
		.addConstant("floatConstant", 3.141592f)
		.addConstant("doubleConstant", 3.141592)
		.addConstant("boolConstant", true)
		.addConstant("stringConstant", std::string("test"))
		.addConstant("enumConstant", TestEnum::TestValue1)
		.addConstant("vectorConstant", TestVector4(2.0f, 4.0f, 6.0f, 8.0f))
		.addConstant("vectorConstantPtr", testVectorConst.get())
		
		.addMember("nestedSelfObject", &ReflectedObject::nestedSelfObject, MF::isWritable)
		.addMember("nullObject", &ReflectedObject::nullObject)
		.addMember("nestedObject", &ReflectedObject::nestedObject)
		.addMember("nestedObjectPointer", &ReflectedObject::nestedObjectPointer, MF::isWritable)
		.addMember("nestedObjectUniquePointer", &ReflectedObject::nestedObjectUniquePointer)
		
		.addMember("objectVector", &ReflectedObject::objectVector)
		.addMember("reflectableObjectsVector", &ReflectedObject::reflectableObjectsVector)
		.addMember("reflectableUniqueObjectsVector", &ReflectedObject::reflectableUniqueObjectsVector)

		.addMember("staticFloat", &ReflectedObject::staticFloat)
		.addMember("staticDouble", &ReflectedObject::staticDouble)
		.addMember("staticInt", &ReflectedObject::staticInt)
		.addMember("staticBool", &ReflectedObject::staticBool)
		.addMember("staticString", &ReflectedObject::staticString)
		.addMember("staticEnum", &ReflectedObject::staticEnum)

		.addMember("staticObject", &ReflectedObject::staticObject)
		.addMember("staticObjectPtr", &ReflectedObject::staticObjectPtr)
		.addMember("staticObjectNullPtr", &ReflectedObject::staticObjectNullPtr)
		.addMember("staticObjectUniquePtr", &ReflectedObject::staticObjectUniquePtr)

		.addMember("staticVector", &ReflectedObject::staticVector)
		.addMember("staticMap", &ReflectedObject::staticMap)
		.addMember("staticStringMap", &ReflectedObject::staticStringMap)

		.addMember("floatVector", &ReflectedObject::floatVector)
		.addMember("boolVector", &ReflectedObject::boolVector)

		.addMember("intToStringMap", &ReflectedObject::intToStringMap)
		.addMember("intToFloatMap", &ReflectedObject::intToFloatMap)

		.addMember("reflectableObjectsUnorderedMap", &ReflectedObject::reflectableObjectsUnorderedMap)
		.addMember("stringToFloatUnorderedMap", &ReflectedObject::stringToFloatUnorderedMap)
		.addMember("reflectableObjectsToBoolUnorderedMap", &ReflectedObject::reflectableObjectsToBoolUnorderedMap)

		.addMember("boolArray", &ReflectedObject::boolArray)
		.addMember("floatArray", &ReflectedObject::floatArray)
		.addMember("objectArray", &ReflectedObject::objectArray)

		.addMember("boolDeque", &ReflectedObject::boolDeque)
		.addMember("intDeque", &ReflectedObject::intDeque)
		.addMember("objectUniquePtrDeque", &ReflectedObject::objectUniquePtrDeque)

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


double ReflectedObject::getDouble()
{
	return aDouble;
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


TestEnum TestObjects::ReflectedObject::getEnum()
{
	return TestEnum::TestValue2;
}


TestVector4 TestObjects::ReflectedObject::getTestVector()
{
	return v1;
}


const TestVector4 TestObjects::ReflectedObject::getConstTestVector() const
{
	return v1;
}


TestVector4& TestObjects::ReflectedObject::getTestVectorRef()
{
	return v1;
}


const TestVector4& TestObjects::ReflectedObject::getTestVectorConstRef() const
{
	return v1;
}


TestVector4* TestObjects::ReflectedObject::getTestVectorPtr()
{
	return &v2;
}


TestVector4 TestObjects::ReflectedObject::addVectors(TestVector4 lhs, TestVector4 rhs)
{
	return lhs + rhs;
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


double ReflectedObject::getConstantDouble() const
{
	return aDouble;
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


TestEnum TestObjects::ReflectedObject::getConstEnum() const
{
	return TestEnum::TestValue3;
}


ReflectedObject* ReflectedObject::getConstObject() const
{
	return nestedSelfObject;
}


void ReflectedObject::doSomethingConst() const
{
	std::cout << "TEST DoingSomethingConst\n";
}


float TestObjects::ReflectedObject::getStaticFloat()
{
	return 42.0f;
}


double TestObjects::ReflectedObject::getStaticDouble()
{
	return 43.0;
}

int TestObjects::ReflectedObject::getStaticInt()
{
	return 42;
}

bool TestObjects::ReflectedObject::getStaticBool()
{
	return true;
}

std::string TestObjects::ReflectedObject::getStaticString()
{
	return "Hi!";
}

TestVector4 TestObjects::ReflectedObject::getStaticObject()
{
	return TestVector4(1.0f, 2.0f, 3.0f, 4.0f);
}


const TestVector4 TestObjects::ReflectedObject::getStaticConstObject()
{
	return TestVector4(4.0f, 3.0f, 2.0f, 1.0f);
}


TestVector4& TestObjects::ReflectedObject::getStaticObjectRef()
{
	static TestVector4 test(9.0f, 8.0f, 7.0f, 6.0f);
	return test;
}


const TestVector4& TestObjects::ReflectedObject::getStaticObjectConstRef()
{
	static TestVector4 test(42.0f, 43.0f, 44.0f, 45.0f);
	return test;
}


TestVector4* TestObjects::ReflectedObject::getStaticObjectPtr()
{
	static TestVector4 test(11.0f, 12.0f, 13.0f, 14.0f);
	return &test;
}


TestEnum TestObjects::ReflectedObject::getStaticEnum()
{
	return TestEnum::TestValue1;
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


float ReflectedObject::staticFloat = 1234.5f;
double ReflectedObject::staticDouble = 7890.1;
int ReflectedObject::staticInt = 33;
bool ReflectedObject::staticBool = true;
std::string ReflectedObject::staticString = "SomeString";
TestEnum ReflectedObject::staticEnum = TestEnum::TestValue3;
const std::unique_ptr<TestVector4> ReflectedObject::testVectorConst = std::make_unique<TestVector4>(4.0f, 4.4f, 4.44f, 4.444f);
NestedReflectedObject ReflectedObject::staticObject = NestedReflectedObject();
NestedReflectedObject* ReflectedObject::staticObjectPtr = new NestedReflectedObject();
NestedReflectedObject* ReflectedObject::staticObjectNullPtr = nullptr;
std::unique_ptr<NestedReflectedObject> ReflectedObject::staticObjectUniquePtr = std::make_unique<NestedReflectedObject>();

std::vector<int> ReflectedObject::staticVector = {42, 11, 0};;
std::map<float, std::string> ReflectedObject::staticMap = {{1.0f, "1.0f"}, {42.0f , "42.0f"}};
std::map<std::string, int> ReflectedObject::staticStringMap = {{"one", 1},{"two", 2}};


