/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ContainerType.h"

#include <algorithm>
#include <iostream>
#include <locale>
#include <string>


const char* toString(ContainerType type)
{
	switch (type)
	{
		default:
		case ContainerType::None:		return "None";
		case ContainerType::Vector:		return "Vector";
		case ContainerType::StringMap:	return "StringMap";
	}
}


ContainerType toContainerType(const char* value)
{
	std::string str(value);
	for (int i = 0; i < (int)ContainerType::Count; i++)
	{
		if (str == toString((ContainerType)i))
		{
			return (ContainerType)i;
		}
	}
	return ContainerType::None;
}
