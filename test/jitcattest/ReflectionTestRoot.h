/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "ReflectionTestObject.h"

#include "Reflectable.h"

#include <memory>

class ReflectionTestRoot: public Reflectable
{
public:
	ReflectionTestRoot();
	~ReflectionTestRoot();

	static void reflect(TypeInfo& typeInfo);
	static const char* getTypeName();

	float getPi() const;

private:
	ReflectionTestObject* testObject;
	ReflectionTestObject testObject2;
	std::unique_ptr<ReflectionTestObject> testObject3;
	float pi;
	int two; 
	std::string hello;
	bool yes;
	bool no;
};
