/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include "jitcat/Reflectable.h"


class ReflectionTestObject2: public jitcat::Reflection::Reflectable
{
public:
	ReflectionTestObject2();

	static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
	static const char* getTypeName();

	std::string getWhat();
	static constexpr bool enableCopyConstructor = true;
public:
	std::string what;
	float aLot;
};