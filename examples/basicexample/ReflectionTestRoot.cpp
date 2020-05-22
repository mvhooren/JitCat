/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ReflectionTestRoot.h"
#include "ReflectionTestObject.h"
#include "jitcat/ReflectedTypeInfo.h"

using namespace jitcat::Reflection;


ReflectionTestRoot::ReflectionTestRoot():
	testObject2("test2 test2 test2"),
	testObject3(new ReflectionTestObject("test3 test3 test3")),
	pi(3.1415926f),
	two(2),
	hello("world"),
	yes(true),
	no(false)
{
	testObject = new ReflectionTestObject("test test test");
	testUnorderedMap[1] = 1.1f;
	testUnorderedMap[2] = 2.2f;
}


ReflectionTestRoot::~ReflectionTestRoot()
{
	delete testObject;
}


float ReflectionTestRoot::getPi() const
{
	return pi;
}

const char* ExternalReflector<ReflectionTestRoot>::getTypeName()
{
	return "Root";
}


void ExternalReflector<ReflectionTestRoot>::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
{
	typeInfo.addMember("test", &ReflectionTestRoot::testObject, MF::isConst);
	typeInfo.addMember("test2", &ReflectionTestRoot::testObject2);
	typeInfo.addMember("test3", &ReflectionTestRoot::testObject3);
	typeInfo.addMember("getPi", &ReflectionTestRoot::getPi);
	typeInfo.addMember("pi", &ReflectionTestRoot::pi);
	typeInfo.addMember("two", &ReflectionTestRoot::two);
	typeInfo.addMember("hello", &ReflectionTestRoot::hello);
	typeInfo.addMember("yes", &ReflectionTestRoot::yes);
	typeInfo.addMember("no", &ReflectionTestRoot::no);
	typeInfo.addMember("testMap", &ReflectionTestRoot::testUnorderedMap);
}
