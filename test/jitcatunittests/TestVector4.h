/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Reflectable.h"
#include <iostream>
#include <catch2/catch.hpp>

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

		TestVector4 doAdd(const TestVector4& other);
		static TestVector4 staticAdd(const TestVector4& a, TestVector4* b, TestVector4 c);

		bool operator==(const TestVector4& other) const;
		TestVector4 operator*(const TestVector4& other) const;
		TestVector4 operator*(int value) const;
		TestVector4 operator*(float value) const;
		TestVector4 operator+(const TestVector4& other) const;
		TestVector4 operator-(const TestVector4& other) const;

		float operator[](int index);

		float x;
		float y;
		float z;
		float w;
		static TestVector4 zero;
		static int instanceCount;
		static bool enableConstructionAndDestructionLogging;
	};

	TestVector4 operator/(const TestVector4& lhs, const TestVector4& rhs);
}

std::ostream& operator<<(std::ostream& os, const TestObjects::TestVector4& vector);

namespace Catch 
{
	template<> struct StringMaker<TestObjects::TestVector4>
	{
		static std::string convert(const TestObjects::TestVector4& vector); 
	};
}