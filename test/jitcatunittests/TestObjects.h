/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "NestedReflectedObject.h"
#include "TestEnum.h"
#include "TestVector4.h"
#include "jitcat/ExternalReflector.h"
#include "jitcat/Reflectable.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTraits.h"

#include <array>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


class TestOptions
{
public:
	static bool enableVerboseLogging;
};


namespace TestObjects
{
	class CaseInsensitiveCompare
	{
	public:
		bool operator() (const std::string& first, const std::string& second) const
		{
			return jitcat::Tools::lessWhileIgnoringCase(first, second);
		}
	};
}

template <>
class jitcat::Reflection::ExternalReflector<TestObjects::CaseInsensitiveCompare>
{
public:
	static const char* getTypeName() {return "Test_CaseInsensitiveCompare";}
	static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo) {}	
	static constexpr bool exists = true;
};


namespace TestObjects
{
	class ReflectedObject: public jitcat::Reflection::Reflectable
	{
	public:
		ReflectedObject();
		~ReflectedObject();
		ReflectedObject(const ReflectedObject&&) = delete;
		ReflectedObject(const ReflectedObject&) = delete;
		ReflectedObject& operator=(const ReflectedObject&) = delete;

		void createNestedObjects();
		void createNullObjects();

		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
		static const char* getTypeName();

		//All supported return types
		float getFloat();
		double getDouble();
		int getInt();
		bool getBoolean();
		std::string getString();
		const std::string& getStringRef();
		TestEnum getEnum();
		TestVector4 getTestVector();
		const TestVector4 getConstTestVector() const;
		TestVector4& getTestVectorRef();
		const TestVector4& getTestVectorConstRef() const;
		TestVector4* getTestVectorPtr();
		TestVector4 addVectors(TestVector4 lhs, TestVector4 rhs);
		ReflectedObject* getObject();

		ReflectedObject* getObject2(const std::string& name, bool amITrue);

		void doSomething();

		//const versions of all supported return types
		float getConstantFloat() const;
		double getConstantDouble() const;
		int getConstInt() const;
		bool getConstBool() const;
		std::string getConstString() const;
		TestEnum getConstEnum() const;
		ReflectedObject* getConstObject() const;
		void doSomethingConst() const;

		static float getStaticFloat();
		static double getStaticDouble();
		static int getStaticInt();
		static bool getStaticBool();
		static std::string getStaticString();
		static TestVector4 getStaticObject();
		static const TestVector4 getStaticConstObject();
		static TestVector4& getStaticObjectRef();
		static const TestVector4& getStaticObjectConstRef();
		static TestVector4* getStaticObjectPtr();
		static TestEnum getStaticEnum();

		//functions for testing parameter passing
		void checkTheseValues(bool amITrue, int someAmount, const std::string& someText, ReflectedObject* someObject);
		std::string returnThisString(const std::string& aString) const;
		std::string addToString(const std::string& text, float number);
		ReflectedObject* getThisObject(ReflectedObject* someObject) const;
	

	public:
		TestVector4 v1;
		TestVector4 v2;
		std::string numberString;
		std::string text;
		int theInt;
		int zeroInt;
		int largeInt;
		float aFloat;
		float negativeFloat;
		float smallFloat;
		float zeroFloat;
		double aDouble;
		double negativeDouble;
		double smallDouble;
		double zeroDouble;
		bool aBoolean;
		bool no;
		TestEnum someEnum;

		ReflectedObject* nestedSelfObject;
		ReflectedObject* nullObject;

		NestedReflectedObject nestedObject;
		NestedReflectedObject* nestedObjectPointer;
		std::unique_ptr<NestedReflectedObject> nestedObjectUniquePointer;

		std::vector<NestedReflectedObject> objectVector;
		std::vector<NestedReflectedObject*> reflectableObjectsVector;
		std::vector<std::unique_ptr<NestedReflectedObject>> reflectableUniqueObjectsVector;
		std::vector<float> floatVector;
		std::vector<bool> boolVector;
		std::map<int, float> intToFloatMap;
		std::map<int, std::string> intToStringMap;
		std::map<std::string, NestedReflectedObject*> reflectableObjectsMap;
		std::map<std::string, NestedReflectedObject*, CaseInsensitiveCompare> reflectableObjectsMapCustomCompare;
		std::map<std::string, std::unique_ptr<NestedReflectedObject>> reflectableUniqueObjectsMap;

		std::unordered_map<int, NestedReflectedObject*> reflectableObjectsUnorderedMap;
		std::unordered_map<std::string, float> stringToFloatUnorderedMap;
		std::unordered_map<NestedReflectedObject*, bool> reflectableObjectsToBoolUnorderedMap;

		std::array<bool, 2> boolArray;
		std::array<float, 2> floatArray;
		std::array<NestedReflectedObject, 2> objectArray;

		std::deque<bool> boolDeque;
		std::deque<int> intDeque;
		std::deque<std::unique_ptr<NestedReflectedObject>> objectUniquePtrDeque;

		static float staticFloat;
		static double staticDouble;
		static int staticInt;
		static bool staticBool;
		static std::string staticString;
		static TestEnum staticEnum;

		static const std::unique_ptr<TestVector4> testVectorConst;

		static NestedReflectedObject staticObject;
		static NestedReflectedObject* staticObjectPtr;
		static NestedReflectedObject* staticObjectNullPtr;
		static std::unique_ptr<NestedReflectedObject> staticObjectUniquePtr;

		static std::vector<int> staticVector;
		static std::map<float, std::string> staticMap;
		static std::map<std::string, int> staticStringMap;
	};


	class VirtualBaseClass
	{
	public:
		VirtualBaseClass(): someMember(11.0f) {};
		virtual ~VirtualBaseClass() {};

		virtual float getStuff() = 0;

		float someMember;
	};

	class MultipleInheritanceClass: public VirtualBaseClass, public jitcat::Reflection::Reflectable 
	{
	public:
		MultipleInheritanceClass(): aMember(42.0f) {};
		virtual ~MultipleInheritanceClass() {};

		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
		static const char* getTypeName();
		virtual float getStuff() override final {return (float)aMember;}
		float getAMember(const std::string& which) const;

		float aMember;
	};
}