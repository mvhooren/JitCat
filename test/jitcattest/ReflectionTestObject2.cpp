/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ReflectionTestObject2.h"
#include "TypeInfo.h"


ReflectionTestObject2::ReflectionTestObject2():
	what("Yeah!"),
	aLot(999999.0f)
{
}


void ReflectionTestObject2::reflect(TypeInfo& typeInfo)
{
	typeInfo.addMember("what", &ReflectionTestObject2::what);
	typeInfo.addMember("aLot", &ReflectionTestObject2::aLot);
}


const char* ReflectionTestObject2::getTypeName()
{
	return "Test2";
}
