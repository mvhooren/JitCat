/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Reflectable.h"


namespace TestObjects
{

	class TestVector4: public jitcat::Reflection::Reflectable
	{
	public:
		TestVector4();
		TestVector4(float x, float y, float z, float w);
		TestVector4(const TestVector4& other);
		TestVector4(const TestVector4&& other) noexcept;
		TestVector4& operator=(const TestVector4& other);
		~TestVector4();

		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
		static const char* getTypeName();

		TestVector4 doAdd(TestVector4& other);
		static TestVector4 staticAdd(TestVector4& a, TestVector4* b, TestVector4 c);

		bool operator==(const TestVector4& other) const;
		TestVector4 operator*(const TestVector4& other);
		TestVector4 operator*(int value);
		TestVector4 operator*(float value);
		TestVector4 operator+(const TestVector4& other);
		TestVector4 operator-(const TestVector4& other);

		float operator[](int index);

		float x;
		float y;
		float z;
		float w;
		static TestVector4 zero;
		static int instanceCount;
	};

	TestVector4 operator/(const TestVector4& lhs, const TestVector4& rhs);

}