/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ReflectionTestObject.h"
#include "ReflectionTestObject2.h"
#include "Tools.h"
#include "TypeInfo.h"

#include <random>


ReflectionTestObject::ReflectionTestObject(std::string text):
	text(text),
	theInt(123),
	aFloat(13.37f)
{
	test2 = new ReflectionTestObject2();
	testObjects.push_back(test2);
	ReflectionTestObject2* bla = new ReflectionTestObject2();
	bla->aLot = 42;
	testObjects.push_back(bla);
	mapObjects["first"] = bla;
	mapObjects["second"] = test2;
	mapObjects["yeah!"] = test2;
}


void ReflectionTestObject::reflect(TypeInfo& typeInfo)
{
	typeInfo.addMember("text", &ReflectionTestObject::text);
	typeInfo.addMember("getTest2", &ReflectionTestObject::getTest2);
	typeInfo.addMember("getAFloat", &ReflectionTestObject::getAFloat);
	typeInfo.addMember("addEleven", &ReflectionTestObject::addEleven);
	typeInfo.addMember("theInt", &ReflectionTestObject::theInt, MTF_IS_CONST);
	typeInfo.addMember("aFloat", &ReflectionTestObject::aFloat, MTF_IS_CONST);
	typeInfo.addMember("test2", &ReflectionTestObject::test2);
	typeInfo.addMember("list", &ReflectionTestObject::testObjects);
	typeInfo.addMember("map", &ReflectionTestObject::mapObjects);
	typeInfo.addMember("getRandomInt", &ReflectionTestObject::getRandomInt);
	typeInfo.addMember("addToString", &ReflectionTestObject::addToString);
}


const char* ReflectionTestObject::getTypeName()
{
	return "Test";
}


int ReflectionTestObject::getRandomInt()
{
	return rand();
}


float ReflectionTestObject::getAFloat()
{
	return aFloat;
}


float ReflectionTestObject::addEleven(float value)
{
	return value + 11.0f;
}


ReflectionTestObject2* ReflectionTestObject::getTest2()
{
	return test2;
}


std::string ReflectionTestObject::addToString(const std::string& text, float number)
{
	return Tools::append(text, number);
}
