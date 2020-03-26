/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


class ReflectionTestObject2;
#include "jitcat/Reflectable.h"
#include <ostream>
#include <map>
#include <vector>


struct TestVector: public jitcat::Reflection::Reflectable
{
	TestVector();
	TestVector(float x, float y, float z, float w);
	TestVector(const TestVector& other);
	TestVector(const TestVector&& other);

	TestVector& operator=(const TestVector& other);
	~TestVector();
	static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
	static const char* getTypeName();

	TestVector operator+(const TestVector& other) const;
	TestVector operator+(int value) const;

	float x;
	float y;
	float z;
	float w;
	static bool enableTracking;
	static int vectorInstances;
};

TestVector operator*(const TestVector& v1, TestVector v2);
std::ostream& operator<< (std::ostream& out, TestVector const& v) ;


class ReflectionTestObject: public jitcat::Reflection::Reflectable
{
public:
	ReflectionTestObject(std::string text);

	static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
	static const char* getTypeName();

	int getRandomInt();
	float getAFloat();
	float addEleven(float value);
	ReflectionTestObject2* getTest2();
	TestVector getTestVector() const;
	TestVector multiply(TestVector lhs, TestVector rhs) const;

	std::string addToString(const std::string& text, float number);

private:
	std::string text;
	int theInt;
	float aFloat;

	TestVector v1;
	TestVector v2;

	ReflectionTestObject2* test2;
	std::vector<ReflectionTestObject2*> testObjects;
	std::map<std::string, ReflectionTestObject2*> mapObjects;
};