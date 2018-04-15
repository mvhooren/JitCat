/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


enum class SpecificMemberType
{
	None,
	CatType,
	NestedType,
	ContainerType,
	Count
};

const char* toString(SpecificMemberType type);
SpecificMemberType toSpecificMemberType(const char* value);