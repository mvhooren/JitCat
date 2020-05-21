/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "TestVector4.h"
#include "jitcat/ReflectedTypeInfo.h"


using namespace jitcat;
using namespace jitcat::Reflection;
using namespace TestObjects;


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
		.template addMember<TestVector4, TestVector4, const TestVector4&>("*", &TestVector4::operator*)
		.template addMember<TestVector4, TestVector4, int>("*", &TestVector4::operator*)
		.template addMember<TestVector4, TestVector4, float>("*", &TestVector4::operator*)
		.addMember("+", &TestVector4::operator+)
		.addMember("-", &TestVector4::operator-)
		.addMember("[]", &TestVector4::operator[])
		.addMember("==", &TestVector4::operator==)
		.addMember("=", &TestVector4::operator=)
		.addMember("zero", &TestVector4::zero)
		.template addMember<TestVector4, const TestVector4&, const TestVector4&>("/", &operator/);

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

TestVector4 TestVector4::zero = TestVector4();
int TestVector4::instanceCount = 0;


TestVector4 TestObjects::operator/(const TestVector4& lhs, const TestVector4& rhs)
{
	return TestVector4(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w);
}
