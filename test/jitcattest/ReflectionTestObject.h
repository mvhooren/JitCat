/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


class ReflectionTestObject2;
#include "Reflectable.h"
#include <map>
#include <vector>


class ReflectionTestObject: public Reflectable
{
public:
	ReflectionTestObject(std::string text);

	static void reflect(TypeInfo& typeInfo);
	static const char* getTypeName();

	int getRandomInt();

	std::string addToString(const std::string& text, float number);

private:
	std::string text;
	int theInt;
	ReflectionTestObject2* test2;
	std::vector<ReflectionTestObject2*> testObjects;
	std::map<std::string, ReflectionTestObject2*> mapObjects;
};