/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatType.h"

#include <cctype>
#include <algorithm>
#include <iostream>
#include <locale>
#include <string>


const char* toString(CatType type)
{
	switch (type)
	{
		case CatType::Int:			return "int";
		case CatType::Float:		return "float";
		case CatType::String:		return "string";
		case CatType::Bool:			return "bool";
		case CatType::Void:			return "void";
		case CatType::Object:		return "object";
		default:
		case CatType::Unknown:		return "unknown";
	}
}


CatType toCatType(const char* value)
{
	std::string str(value);
	std::size_t length = str.length();
	for (std::size_t i = 0; i < length; i++)
	{
		str[i] = std::tolower(str[i], std::locale());
	}
	for (int i = 0; i < (int)CatType::Count; i++)
	{
		if (str == toString((CatType)i))
		{
			return (CatType)i;
		}
	}
	if (str == "custom")
	{
		return CatType::Object;
	}
	return CatType::Unknown;
}


bool isScalar(CatType type)
{
	return type == CatType::Int || type == CatType::Float;
}


bool isBasicType(CatType type)
{
	return type == CatType::Int || type == CatType::Float || type == CatType::String || type == CatType::Bool;
}