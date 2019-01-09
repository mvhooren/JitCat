/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "Reflectable.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TestObjects
{

	class NestedReflectedObject: public Reflectable
	{
	public:
		NestedReflectedObject();

		static void reflect(TypeInfo& typeInfo);
		static const char* getTypeName();

		public:
			std::string someString;
			int someInt;
			float someFloat;
			bool someBoolean;
			NestedReflectedObject* nullObject;
	};


	class ReflectedObject: public Reflectable
	{
	public:
		ReflectedObject();
		void createNestedObjects();
		void createNullObjects();

		static void reflect(TypeInfo& typeInfo);
		static const char* getTypeName();

		//All supported return types
		float getFloat();
		int getInt();
		bool getBoolean();
		std::string getString();
		ReflectedObject* getObject();
		void doSomething();

		//const versions of all supported return types
		float getConstantFloat() const;
		int getConstInt() const;
		bool getConstBool() const;
		std::string getConstString() const;
		ReflectedObject* getConstObject() const;
		void doSomethingConst() const;

		//functions for testing parameter passing
		void checkTheseValues(bool amITrue, int someAmount, const std::string& someText, ReflectedObject* someObject);
		std::string returnThisString(const std::string& aString) const;
		std::string addToString(const std::string& text, float number);
		ReflectedObject* getThisObject(ReflectedObject* someObject) const;
	

	public:
		std::string numberString;
		std::string text;
		int theInt;
		int largeInt;
		float aFloat;
		float zeroFloat;
		bool aBoolean;
		bool no;

		ReflectedObject* nestedSelfObject;

		NestedReflectedObject nestedObject;
		NestedReflectedObject* nestedObjectPointer;
		std::unique_ptr<NestedReflectedObject> nestedObjectUniquePointer;

		std::vector<NestedReflectedObject*> reflectableObjectsVector;
		std::map<std::string, NestedReflectedObject*> reflectableObjectsMap;
	};
}