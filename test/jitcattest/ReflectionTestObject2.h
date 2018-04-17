/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include "Reflectable.h"


class ReflectionTestObject2: public Reflectable
{
public:
	ReflectionTestObject2();

	static void reflect(TypeInfo& typeInfo);
	static const char* getTypeName();

	std::string getWhat();

public:
	std::string what;
	float aLot;
};