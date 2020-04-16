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
	nullCircularRefObject(nullptr),
	someV4(1.0, 2.0f, 3.0f, 4.0f)
{
}


void NestedReflectedObject::reflect(ReflectedTypeInfo& typeInfo)
{
	typeInfo
		.addMember("someString", &NestedReflectedObject::someString)
		.addMember("someInt", &NestedReflectedObject::someInt)
		.addMember("someFloat", &NestedReflectedObject::someFloat)
		.addMember("someBoolean", &NestedReflectedObject::someBoolean)
		.addMember("someV4", &NestedReflectedObject::someV4)
		.addMember("nullObject", &NestedReflectedObject::nullObject)
		.addMember("nullCircularRefObject", &NestedReflectedObject::nullCircularRefObject, MF::isWritable)
		.addMember("emptyCircularRefList", &NestedReflectedObject::emptyCircularRefList);
}


const char* NestedReflectedObject::getTypeName()
{
	return "NestedReflectedObject";
}


bool TestObjects::NestedReflectedObject::operator==(const NestedReflectedObject& other) const
{
	return someString == other.someString 
			&& someInt == other.someInt 
			&& someFloat == other.someFloat 
			&& someBoolean == other.someBoolean 
			&& nullObject == other.nullObject 
			&& nullCircularRefObject == other.nullCircularRefObject
			&& someV4 == other.someV4
			&& emptyCircularRefList == other.emptyCircularRefList;
}


ReflectedObject::ReflectedObject():
	nestedSelfObject(nullptr),
	nestedObjectPointer(nullptr),
	nestedObjectUniquePointer(nullptr),
	nullObject(nullptr),
	text("Hello!"),
	numberString("123.4"),
	theInt(42),
	zeroInt(0),
	largeInt(1234567),
	aFloat(999.9f),
	negativeFloat(-111.1f),
	smallFloat(0.5f),
	zeroFloat(0.0f),
	aBoolean(true),
	no(false),
	v1(1.0f, 2.0f, 3.0f, 4.0f),
	v2(4.0f, 3.0f, 2.0f, 1.0f)

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
	nestedObjectUniquePointer = std::make_unique<NestedReflectedObject>();

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
		.addMember("getTestVector", &ReflectedObject::getTestVector)
		.addMember("getTestVectorRef", &ReflectedObject::getTestVectorRef)
		.addMember("getTestVectorConstRef", &ReflectedObject::getTestVectorConstRef)
		.addMember("getTestVectorPtr", &ReflectedObject::getTestVectorPtr)
		.addMember("getConstTestVector", &ReflectedObject::getConstTestVector)
		.addMember("v1", &ReflectedObject::v1)
		.addMember("v2", &ReflectedObject::v2)
		.addMember("addVectors", &ReflectedObject::addVectors)

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

		.addMember("getStaticFloat", &ReflectedObject::getStaticFloat)
		.addMember("getStaticInt", &ReflectedObject::getStaticInt)
		.addMember("getStaticBool", &ReflectedObject::getStaticBool)
		.addMember("getStaticString", &ReflectedObject::getStaticString)
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


float TestObjects::ReflectedObject::getStaticFloat()
{
	return 42.0f;
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


TestObjects::TestVector4::TestVector4():
	x(0.0f),
	y(0.0f),
	z(0.0f),
	w(0.0f)
{
	instanceCount++;
}


TestObjects::TestVector4::TestVector4(float x, float y, float z, float w):
	x(x),
	y(y),
	z(z),
	w(w)
{
	instanceCount++;
}


TestObjects::TestVector4::TestVector4(const TestVector4& other):
	x(other.x),
	y(other.y),
	z(other.z),
	w(other.w)
{
	instanceCount++;
}


TestObjects::TestVector4::TestVector4(const TestVector4&& other) noexcept:
	x(other.x),
	y(other.y),
	z(other.z),
	w(other.w)
{
	instanceCount++;
}


TestVector4& TestObjects::TestVector4::operator=(const TestVector4& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
	return *this;
}


TestObjects::TestVector4::~TestVector4()
{
	instanceCount--;
}


void TestObjects::TestVector4::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
{
	typeInfo
		.addMember("x", &TestVector4::x)
		.addMember("y", &TestVector4::y)
		.addMember("z", &TestVector4::z)
		.addMember("w", &TestVector4::w)
		.addMember("doAdd", &TestVector4::doAdd)
		.addMember("staticAdd", &TestVector4::staticAdd)
		.addMember<TestVector4, TestVector4, const TestVector4&>("*", &TestVector4::operator*)
		.addMember<TestVector4, TestVector4, int>("*", &TestVector4::operator*)
		.addMember<TestVector4, TestVector4, float>("*", &TestVector4::operator*)
		.addMember("+", &TestVector4::operator+)
		.addMember("-", &TestVector4::operator-)
		.addMember("[]", &TestVector4::operator[])
		.addMember("==", &TestVector4::operator==)
		.addMember<TestVector4, const TestVector4&, const TestVector4&>("/", &operator/);

}


const char* TestObjects::TestVector4::getTypeName()
{
	return "TestVector4";
}


TestVector4 TestObjects::TestVector4::doAdd(TestVector4& other)
{
	return *this + other;
}


TestVector4 TestObjects::TestVector4::staticAdd(TestVector4& a, TestVector4* b, TestVector4 c)
{
	return a + *b + c;
}


bool TestObjects::TestVector4::operator==(const TestVector4& other) const
{
	return x == other.x && y == other.y && z == other.z && w == other.w;
}


TestVector4 TestObjects::TestVector4::operator*(const TestVector4& other)
{
	return TestVector4(x * other.x, y * other.y, z * other.z, w * other.w);
}


TestVector4 TestObjects::TestVector4::operator*(int value)
{
	return TestVector4(x * value, y * value, z * value, w * value);
}


TestVector4 TestObjects::TestVector4::operator*(float value)
{
	return TestVector4(x * value, y * value, z * value, w * value);
}


TestVector4 TestObjects::TestVector4::operator+(const TestVector4& other)
{
	return TestVector4(x + other.x, y + other.y, z + other.z, w + other.w);
}


TestVector4 TestObjects::TestVector4::operator-(const TestVector4& other)
{
	return TestVector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

float TestObjects::TestVector4::operator[](int index)
{
	if (index >= 0 && index < 4)
	{
		return (&x)[index];
	}
	else
	{
		static float zero = 0.0f;
		zero = 0.0f;
		return zero;
	}
}


int TestObjects::TestVector4::instanceCount = 0;


TestVector4 TestObjects::operator/(const TestVector4& lhs, const TestVector4& rhs)
{
	return TestVector4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
}
