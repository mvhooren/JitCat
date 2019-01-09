/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatGenericType.h"
#include "CatType.h"
#include "CatValue.h"
#include "Tools.h"
#include "TypeRegistry.h"

#include <map>
#include <memory>
#include <string>
#include <vector>


//These classes use template specialization to get properties of types relevant for the reflection, serialisation and expression system.
//It allows to translate a type T to a CatType and to check if a T is a reflectable/serialisable container.
//The top class is the default case where T is neither a basic type nor a container type.
//All other classes are specializations for specific types.
template <typename T>
class TypeTraits
{
public:
	static CatType getCatType() { return CatType::Object; }
	static inline CatGenericType toGenericType();
	static bool isSerialisableContainer() { return false; }
	static const char* getTypeName() { return T::getTypeName(); }
	static inline T* getValueFromMemberReference(MemberReference* value);
	static CatValue getCatValue(void) { return CatValue();}
	static CatValue getCatValue(const T& value);
	static T* getValue(const CatValue& value)  { return TypeTraits<T>::getValueFromMemberReference(value.getCustomTypeValue().getPointer());}
	static TypeInfo* getTypeInfo() {return TypeRegistry::get()->registerType<T>();}

	typedef T type;
	typedef T cachedType;
};


template <>
class TypeTraits<void>
{
public:
	static CatType getCatType() { return CatType::Void; }
	static CatGenericType toGenericType() { return CatGenericType(CatType::Void); }
	static bool isSerialisableContainer() { return false; }
	template <typename U>
	static CatValue getCatValue(const U& param) { return CatValue();}
	//static CatValue getCatValue(int) { return CatValue();}

	static void getValue() { return;}
	static void getValue(const CatValue& value) { return;}

	static const char* getTypeName()
	{
		return "void"; 
	}


	typedef void type;
	typedef int cachedType;
};


template <>
class TypeTraits<float>
{
public:
	static CatType getCatType() { return CatType::Float; }
	static CatGenericType toGenericType() { return CatGenericType(CatType::Float); }
	static bool isSerialisableContainer() { return false; }
	static CatValue getCatValue(float value) { return CatValue(value);}
	static float getValue(const CatValue& value) { return value.getFloatValue();}
	static const char* getTypeName()
	{
		return "float"; 
	}
	static float getValueFromMemberReference(MemberReference* value); 


	typedef float type;
	typedef float cachedType;
};


template <>
class TypeTraits<int>
{
public:
	static CatType getCatType() { return CatType::Int; }
	static CatGenericType toGenericType() { return CatGenericType(CatType::Int); }
	static bool isSerialisableContainer() { return false; }
	static CatValue getCatValue(int value) { return CatValue(value);}
	static int getValue(const CatValue& value) { return value.getIntValue();}
	static const char* getTypeName()
	{
		return "int";
	}
	static int getValueFromMemberReference(MemberReference* value);

	typedef int type;
	typedef int cachedType;
};


template <>
class TypeTraits<bool>
{
public:
	static CatType getCatType() { return CatType::Bool; }
	static CatGenericType toGenericType() { return CatGenericType(CatType::Bool); }
	static bool isSerialisableContainer() { return false; }
	static CatValue getCatValue(bool value) { return CatValue(value);}
	static bool getValue(const CatValue& value) { return value.getBoolValue();}
	static const char* getTypeName()
	{
		return "bool";
	}
	static bool getValueFromMemberReference(MemberReference* value);

	typedef bool type;
	typedef bool cachedType;
};


template <>
class TypeTraits<std::string>
{
public:
	static CatType getCatType() { return CatType::String; }
	static CatGenericType toGenericType() { return CatGenericType(CatType::String); }
	static bool isSerialisableContainer() { return false; }
	static CatValue getCatValue(const std::string& value) { return CatValue(value);}
	static std::string getValue(const CatValue& value) { return value.getStringValue();}
	static const char* getTypeName()
	{
		return "string";
	}
	static std::string getValueFromMemberReference(MemberReference* value);

	typedef std::string type;
	typedef std::string cachedType;
};


template <typename U>
class TypeTraits<std::unique_ptr<U>>
{
public:
	static CatType getCatType() { return CatType::Object; }
	static CatGenericType toGenericType();
	static bool isSerialisableContainer() { return false; }
	static const char* getTypeName() { return U::getTypeName(); }
	static CatValue getCatValue(std::unique_ptr<U>& value);
	static U* getValue(const CatValue& value) { return TypeTraits<std::unique_ptr<U>>::getValueFromMemberReference(value.getCustomTypeValue().getPointer());}
	static inline U* getValueFromMemberReference(MemberReference* value);
	static U* getPointer(std::unique_ptr<U>& value);
	static TypeInfo* getTypeInfo() {return TypeRegistry::get()->registerType<U>();}

	typedef U type;
	typedef U* cachedType;
};


template <typename U>
class TypeTraits<U*>
{
public:
	static CatType getCatType() { return CatType::Object; }
	static CatGenericType toGenericType();
	static bool isSerialisableContainer() { return false; }
	static const char* getTypeName() { return U::getTypeName(); }
	static CatValue getCatValue(U* value);
	static U* getValue(const CatValue& value) { return TypeTraits<U*>::getValueFromMemberReference(value.getCustomTypeValue().getPointer());}
	static U* getPointer(U* value) {return value;};
	static inline U* getValueFromMemberReference(MemberReference* value);
	static TypeInfo* getTypeInfo() {return TypeRegistry::get()->registerType<U>();}

	typedef U type;
	typedef U* cachedType;
};


template <typename ItemType>
class TypeTraits<std::vector<ItemType> >
{
public:
	static CatType getCatType() { return CatType::Unknown; }
	static CatGenericType toGenericType();
	static bool isSerialisableContainer() { return true; }
	static const char* getTypeName() { return ""; }
	static CatValue getCatValue(void) { return CatValue();}
	static std::vector<ItemType>& getValue(const CatValue& value) { return TypeTraits<std::vector<ItemType>>::getValueFromMemberReference(value.getCustomTypeValue().getPointer());}
	static inline std::vector<ItemType>& getValueFromMemberReference(MemberReference* value);

	typedef ItemType type;
	typedef std::vector<ItemType> cachedType;
};


template <typename ItemType>
class TypeTraits<std::map<std::string, ItemType> >
{
public:
	static CatType getCatType() { return CatType::Unknown; }
	static CatGenericType toGenericType();
	static bool isSerialisableContainer() { return true; }
	static const char* getTypeName() { return ""; }
	static CatValue getCatValue(void) { return CatValue();}
	static std::map<std::string, ItemType>& getValue(const CatValue& value) { return TypeTraits<std::map<std::string, ItemType>>::getValueFromMemberReference(value.getCustomTypeValue().getPointer());}
	static inline std::map<std::string, ItemType>& getValueFromMemberReference(MemberReference* value);

	typedef ItemType type;
	typedef std::map<std::string, ItemType> cachedType;
};

#include "TypeTraitsHeaderImplementation.h"
