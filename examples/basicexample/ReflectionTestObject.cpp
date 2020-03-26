/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ReflectionTestObject.h"
#include "ReflectionTestObject2.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/Tools.h"

#include <iostream>
#include <random>

using namespace jitcat;
using namespace jitcat::Reflection;


ReflectionTestObject::ReflectionTestObject(std::string text):
	text(text),
	theInt(123),
	aFloat(13.37f),
	v1(1.0f, 2.0f, 3.0f, 4.0f),
	v2(4.0f, 3.0f, 2.0f, 1.0f)
{
	test2 = new ReflectionTestObject2();
	testObjects.push_back(test2);
	ReflectionTestObject2* bla = new ReflectionTestObject2();
	bla->aLot = 42;
	testObjects.push_back(bla);
	mapObjects["first"] = bla;
	mapObjects["second"] = test2;
	mapObjects["yeah!"] = test2;
}


void ReflectionTestObject::reflect(ReflectedTypeInfo& typeInfo)
{
	typeInfo.addMember("text", &ReflectionTestObject::text);
	typeInfo.addMember("v1", &ReflectionTestObject::v1);
	typeInfo.addMember("v2", &ReflectionTestObject::v2);
	typeInfo.addMember("getTest2", &ReflectionTestObject::getTest2);
	typeInfo.addMember("getAFloat", &ReflectionTestObject::getAFloat);
	typeInfo.addMember("getTestVector", &ReflectionTestObject::getTestVector);
	typeInfo.addMember("multiply", &ReflectionTestObject::multiply);
	typeInfo.addMember("addEleven", &ReflectionTestObject::addEleven);
	typeInfo.addMember("theInt", &ReflectionTestObject::theInt, MF::isConst);
	typeInfo.addMember("aFloat", &ReflectionTestObject::aFloat, MF::isConst);
	typeInfo.addMember("test2", &ReflectionTestObject::test2);
	typeInfo.addMember("list", &ReflectionTestObject::testObjects);
	typeInfo.addMember("map", &ReflectionTestObject::mapObjects);
	typeInfo.addMember("getRandomInt", &ReflectionTestObject::getRandomInt);
	typeInfo.addMember("addToString", &ReflectionTestObject::addToString);
}


const char* ReflectionTestObject::getTypeName()
{
	return "Test";
}


int ReflectionTestObject::getRandomInt()
{
	return rand();
}


float ReflectionTestObject::getAFloat()
{
	return aFloat;
}


float ReflectionTestObject::addEleven(float value)
{
	return value + 11.0f;
}


ReflectionTestObject2* ReflectionTestObject::getTest2()
{
	return test2;
}


TestVector ReflectionTestObject::getTestVector() const
{
	return v1;
}


TestVector ReflectionTestObject::multiply(TestVector lhs, TestVector rhs) const
{
	return lhs * rhs;
}


std::string ReflectionTestObject::addToString(const std::string& text, float number)
{
	return Tools::append(text, number);
}


TestVector::TestVector():
	x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{
	if (enableTracking)
	{
		vectorInstances++;
		std::cout << "Default Contructed: " << this << "\n";
	}
}


TestVector::TestVector(float x, float y, float z, float w):
	x(x),
	y(y),
	z(z),
	w(w)
{
	if (enableTracking)
	{
		vectorInstances++;
		std::cout << "Contructed: " << this << "\n";
	}
}


TestVector::TestVector(const TestVector& other):
	x(other.x),
	y(other.y),
	z(other.z),
	w(other.w)
{
	if (enableTracking)
	{
		vectorInstances++;
		std::cout << "Copy Contructed: " << this << "\n";
	}
}


TestVector::TestVector(const TestVector&& other):
	x(other.x),
	y(other.y),
	z(other.z),
	w(other.w)
{
	if (enableTracking)
	{
		vectorInstances++;
		std::cout << "Move Contructed: " << this << "\n";
	}
}


TestVector& TestVector::operator=(const TestVector& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	w = other.w;
	return *this;
}


TestVector::~TestVector()
{
	if (enableTracking)
	{
		vectorInstances--;
		std::cout << "Deleted: " << this << "\n";
	}
}

int TestVector::vectorInstances = 0;
bool TestVector::enableTracking = false;;
void TestVector::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
{
	typeInfo
		.addMember("x", &TestVector::x, MemberFlags::isWritable)
		.addMember("y", &TestVector::y, MemberFlags::isWritable)
		.addMember("z", &TestVector::z, MemberFlags::isWritable)
		.addMember("w", &TestVector::w, MemberFlags::isWritable)
		.addMember<TestVector, TestVector, const TestVector&>("+", &TestVector::operator+)
		.addMember<TestVector, TestVector, int>("+", &TestVector::operator+)
		.addMember<TestVector, const TestVector&, TestVector>("*", &::operator*);
}


const char* TestVector::getTypeName()
{
	return "TestVector";
}


TestVector TestVector::operator+(const TestVector& other) const
{
	return TestVector(x + other.x, y + other.y, z + other.z, w + other.w);
}


TestVector TestVector::operator+(int value) const
{
	return TestVector(x + value, y + value, z + value, w + value);
}


TestVector operator*(const TestVector& v1, TestVector v2)
{
	return TestVector(v1.x * v2.x, 
					  v1.y * v2.y, 
					  v1.z * v2.z, 
					  v1.w * v2.w);
}

std::ostream& operator<<(std::ostream& out, TestVector const& v)
{
	out << "x: " << v.x << " y: " << v.y << " z: " << v.z << " w: " << v.w;
    return out;
}
