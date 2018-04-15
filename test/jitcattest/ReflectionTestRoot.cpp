/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ReflectionTestRoot.h"
#include "ReflectionTestObject.h"
#include "TypeInfo.h"


ReflectionTestRoot::ReflectionTestRoot():
	pi(3.1415926f),
	two(2),
	hello("world"),
	yes(true),
	no(false),
	testObject2("test2 test2 test2"),
	testObject3(new ReflectionTestObject("test3 test3 test3"))
{
	testObject = new ReflectionTestObject("test test test");
}


ReflectionTestRoot::~ReflectionTestRoot()
{
	delete testObject;
}


void ReflectionTestRoot::reflect(TypeInfo& typeInfo)
{
	typeInfo.addMember("test", &ReflectionTestRoot::testObject, MTF_IS_CONST);
	typeInfo.addMember("test2", &ReflectionTestRoot::testObject2);
	typeInfo.addMember("test3", &ReflectionTestRoot::testObject3);
	typeInfo.addMember("pi", &ReflectionTestRoot::pi);
	typeInfo.addMember("two", &ReflectionTestRoot::two);
	typeInfo.addMember("hello", &ReflectionTestRoot::hello);
	typeInfo.addMember("yes", &ReflectionTestRoot::yes);
	typeInfo.addMember("no", &ReflectionTestRoot::no);
}


const char* ReflectionTestRoot::getTypeName()
{
	return "Root";
}
