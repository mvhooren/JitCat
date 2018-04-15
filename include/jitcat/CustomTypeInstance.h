/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CustomTypeInfo;
#include "Reflectable.h"

//Represents an instance of a custom defined type.
//The type is defined by the CustomTypeInfo
//The CustomTypeInfo will create the memory for holding the members. This is stored in data.
//When the instance is destroyed, the data will also be destructud by CustomTypeInfo
class CustomTypeInstance: public Reflectable
{
protected:
	friend class CustomTypeInfo;
	CustomTypeInstance(CustomTypeInfo* typeInfo);
	CustomTypeInstance(unsigned char* data, CustomTypeInfo* typeInfo);
	CustomTypeInstance(const CustomTypeInstance& typeInfo) = delete;

public:
	virtual ~CustomTypeInstance();

	CustomTypeInfo* typeInfo;
	unsigned char* data;
};