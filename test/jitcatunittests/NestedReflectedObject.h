/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "TestVector4.h"


namespace TestObjects
{
	class ReflectedObject;

	class NestedReflectedObject: public jitcat::Reflection::Reflectable
	{
	public:
		NestedReflectedObject();

		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
		static const char* getTypeName();

		bool operator==(const NestedReflectedObject& other) const;

	public:
		std::string someString;
		int someInt;
		float someFloat;
		bool someBoolean;
		NestedReflectedObject* nullObject;
		TestVector4 someV4;
		//Test for circular reference
		ReflectedObject* nullCircularRefObject;
		std::vector<ReflectedObject*> emptyCircularRefList;
	};

}