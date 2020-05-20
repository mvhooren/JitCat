/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "NestedReflectedObject.h"
#include "TestObjects.h"
#include "jitcat/ReflectedTypeInfo.h"


using namespace jitcat;
using namespace jitcat::Reflection;
using namespace TestObjects;


NestedReflectedObject::NestedReflectedObject():
	someString("test"),
	someInt(21),
	someFloat(1.1f),
	someDouble(1.1),
	someBoolean(true),
	nullObject(nullptr),
	nullCircularRefObject(nullptr),
	someV4(1.0, 2.0f, 3.0f, 4.0f)
{
}


void NestedReflectedObject::reflect(ReflectedTypeInfo& typeInfo)
{
	typeInfo
		.addMember("someString", &NestedReflectedObject::someString)
		.addMember("someInt", &NestedReflectedObject::someInt)
		.addMember("someFloat", &NestedReflectedObject::someFloat)
		.addMember("someDouble", &NestedReflectedObject::someDouble)
		.addMember("someBoolean", &NestedReflectedObject::someBoolean)
		.addMember("someV4", &NestedReflectedObject::someV4)
		.addMember("nullObject", &NestedReflectedObject::nullObject)
		.addMember("nullCircularRefObject", &NestedReflectedObject::nullCircularRefObject, MF::isWritable)
		.addMember("emptyCircularRefList", &NestedReflectedObject::emptyCircularRefList);
}


const char* NestedReflectedObject::getTypeName()
{
	return "NestedReflectedObject";
}


bool TestObjects::NestedReflectedObject::operator==(const NestedReflectedObject& other) const
{
	return someString == other.someString 
			&& someInt == other.someInt 
			&& someFloat == other.someFloat 
			&& someBoolean == other.someBoolean 
			&& nullObject == other.nullObject 
			&& nullCircularRefObject == other.nullCircularRefObject
			&& someV4 == other.someV4
			&& emptyCircularRefList == other.emptyCircularRefList;
}