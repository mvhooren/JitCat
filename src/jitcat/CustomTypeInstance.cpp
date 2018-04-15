/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CustomTypeInstance.h"
#include "CustomTypeInfo.h"


CustomTypeInstance::CustomTypeInstance(CustomTypeInfo* typeInfo):
	typeInfo(typeInfo)
{
	data = typeInfo->instanceConstructor();
}


CustomTypeInstance::CustomTypeInstance(unsigned char* data, CustomTypeInfo* typeInfo):
	data(data),
	typeInfo(typeInfo)
{
}


CustomTypeInstance::~CustomTypeInstance()
{
	typeInfo->instanceDestructor(this);
}
