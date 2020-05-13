/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "ReflectionTestObject.h"

#include "jitcat/ExternalReflector.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/Reflectable.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTraits.h"

#include <unordered_map>
#include <memory>

class ReflectionTestRoot: public jitcat::Reflection::Reflectable
{
public:
	ReflectionTestRoot();
	~ReflectionTestRoot();

	float getPi() const;

public:
	ReflectionTestObject* testObject;
	ReflectionTestObject testObject2;
	std::unique_ptr<ReflectionTestObject> testObject3;
	std::unordered_map<int, float> testUnorderedMap;
	float pi;
	int two; 
	std::string hello;
	bool yes;
	bool no;
};

         
//Example of reflection external of the reflected class
template <>
class jitcat::Reflection::ExternalReflector<ReflectionTestRoot>
{
public:
	static const char* getTypeName();
	static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);

	static constexpr bool exists = true;
};
