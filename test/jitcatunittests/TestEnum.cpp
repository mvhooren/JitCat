/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#include "TestEnum.h"


using namespace jitcat;
using namespace jitcat::Reflection;
using namespace TestObjects;

template<>
void jitcat::Reflection::reflectEnum<TestObjects::TestEnum>(jitcat::Reflection::ReflectedEnumTypeInfo& enumTypeInfo)
{
	enumTypeInfo
		.addValue("TestValue1", TestEnum::TestValue1)
		.addValue("TestValue2", TestEnum::TestValue2)
		.addValue("TestValue3", TestEnum::TestValue3)
		.setDefaultValue(TestEnum::TestValue1);

}


template <>
const char* jitcat::Reflection::getEnumName<TestObjects::TestEnum>()
{
	return "TestEnum";
}