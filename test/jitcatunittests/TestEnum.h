/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ReflectedEnumTypeInfo.h"


namespace TestObjects
{
	enum class TestEnum
	{
		TestValue1,
		TestValue2,
		TestValue3
	};
}

template <>
void jitcat::Reflection::reflectEnum<TestObjects::TestEnum>(jitcat::Reflection::ReflectedEnumTypeInfo& enumTypeInfo);

template <>
const char* jitcat::Reflection::getEnumName<TestObjects::TestEnum>();