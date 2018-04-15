/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "SpecificMemberType.h"

#include <algorithm>
#include <iostream>
#include <locale>
#include <string>


const char* toString(SpecificMemberType type)
{
	switch (type)
	{
		default:
		case SpecificMemberType::None:			return "None";
		case SpecificMemberType::CatType:		return "BasicType";
		case SpecificMemberType::NestedType:	return "ObjectType";
		case SpecificMemberType::ContainerType:	return "ContainerType";
	}
}


SpecificMemberType toSpecificMemberType(const char* value)
{
	std::string str(value);
	for (int i = 0; i < (int)SpecificMemberType::Count; i++)
	{
		if (str == toString((SpecificMemberType)i))
		{
			return (SpecificMemberType)i;
		}
	}
	return SpecificMemberType::None;
}