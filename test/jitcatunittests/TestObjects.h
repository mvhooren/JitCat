/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Reflectable.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTraits.h"

#include <map>
#include <memory>
#include <string>
#include <vector>


namespace TestObjects
{
	template<typename ItemType, typename AllocatorType = std::allocator<ItemType>>
	class ReflectableVector: public jitcat::Reflection::Reflectable
	{
	public:
		ReflectableVector() {};
		/*inline operator std::vector<ItemType, AllocatorType>(){return *this;};*/

		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
		static const char* getTypeName();

		void push_back(ItemType&& item);
		void push_back(const ItemType& item);
		template <class... Args>
		void emplace_back(Args&&... argValues) {data.emplace_back(std::forward(argValues)...);}

		ItemType& operator[](int index);

		int sizeAsInt() {return (int)data.size();}
	private:
		std::vector<ItemType, AllocatorType> data;
	};


	template<typename ItemType, typename AllocatorType>
	inline void ReflectableVector<ItemType, AllocatorType>::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
	{
		typeInfo
			.addMember<ReflectableVector<ItemType, AllocatorType>, ItemType&, int>("[]", &ReflectableVector::operator[])
			.addMember("size", &ReflectableVector::sizeAsInt);
	}


	template<typename ItemType, typename AllocatorType>
	inline const char* ReflectableVector<ItemType, AllocatorType>::getTypeName()
	{
		static std::string itemTypeName = jitcat::Tools::append(TypeTraits<ItemType>::getTypeName(), std::is_pointer<ItemType>::value ? "*" : "");
		static std::string vectorName = jitcat::Tools::append("Vector<", itemTypeName, ">");
		return vectorName.c_str();
	}

	
	template<typename ItemType, typename AllocatorType>
	inline void ReflectableVector<ItemType, AllocatorType>::push_back(ItemType&& item)
	{
		data.push_back(item);
	}


	template<typename ItemType, typename AllocatorType>
	inline void ReflectableVector<ItemType, AllocatorType>::push_back(const ItemType& item)
	{
		data.push_back(item);
	}


	template<typename ItemType, typename AllocatorType>
	inline ItemType& ReflectableVector<ItemType, AllocatorType>::operator[](int index)
	{
		if (index >= 0 && index < sizeAsInt())
		{
			return data[index];
		}
		else
		{
			static ItemType default = ItemType();
			return default;
		}
	}


	class CaseInsensitiveCompare
	{
	public:
		bool operator() (const std::string& first, const std::string& second) const
		{
			return jitcat::Tools::lessWhileIgnoringCase(first, second);
		}
	};

	class ReflectedObject;

	class NestedReflectedObject: public jitcat::Reflection::Reflectable
	{
	public:
		NestedReflectedObject();

		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
		static const char* getTypeName();

		public:
			std::string someString;
			int someInt;
			float someFloat;
			bool someBoolean;
			NestedReflectedObject* nullObject;
			//Test for circular reference
			ReflectedObject* nullCircularRefObject;
			std::vector<ReflectedObject*> emptyCircularRefList;
	};


	class ReflectedObject: public jitcat::Reflection::Reflectable
	{
	public:
		ReflectedObject();
		~ReflectedObject();
		void createNestedObjects();
		void createNullObjects();

		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
		static const char* getTypeName();

		//All supported return types
		float getFloat();
		int getInt();
		bool getBoolean();
		std::string getString();
		const std::string& getStringRef();
		ReflectedObject* getObject();
		ReflectedObject* getObject2(const std::string& name, bool amITrue);

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
		int zeroInt;
		int largeInt;
		float aFloat;
		float negativeFloat;
		float smallFloat;
		float zeroFloat;
		bool aBoolean;
		bool no;

		ReflectedObject* nestedSelfObject;
		ReflectedObject* nullObject;

		NestedReflectedObject nestedObject;
		NestedReflectedObject* nestedObjectPointer;
		std::unique_ptr<NestedReflectedObject> nestedObjectUniquePointer;

		std::vector<NestedReflectedObject> objectVector;
		std::vector<NestedReflectedObject*> reflectableObjectsVector;
		std::vector<std::unique_ptr<NestedReflectedObject>> reflectableUniqueObjectsVector;
		std::vector<float> floatVector;
		std::map<int, std::string> intToStringMap;
		std::map<std::string, NestedReflectedObject*> reflectableObjectsMap;
		std::map<std::string, NestedReflectedObject*, CaseInsensitiveCompare> reflectableObjectsMapCustomCompare;
		std::map<std::string, std::unique_ptr<NestedReflectedObject>> reflectableUniqueObjectsMap;
	};
}