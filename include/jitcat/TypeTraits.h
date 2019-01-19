/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatGenericType.h"
#include "CatType.h"
#include "Tools.h"
#include "TypeRegistry.h"

#include <any>
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
	static std::any getCatValue(void) { return std::any();}
	static std::any getCatValue(const T& value);
	static T* getValue(const std::any& value)  { return TypeTraits<T>::getValueFromMemberReference(value.getCustomTypeValue().getPointer());}
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
	static std::any getCatValue(const U& param) { return std::any();}

	static void getValue() { return;}
	static void getValue(const std::any& value) { return;}

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
	static std::any getCatValue(float value) { return std::any(value);}
	static float getValue(const std::any& value) { return std::any_cast<float>(value);}
	static const char* getTypeName()
	{
		return "float"; 
	}


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
	static std::any getCatValue(int value) { return std::any(value);}
	static int getValue(const std::any& value) { return std::any_cast<int>(value);}
	static const char* getTypeName()
	{
		return "int";
	}

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
	static std::any getCatValue(bool value) { return std::any(value);}
	static bool getValue(const std::any& value) { return std::any_cast<bool>(value);}
	static const char* getTypeName()
	{
		return "bool";
	}

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
	static std::any getCatValue(const std::string& value) { return std::any(value);}
	static std::string getValue(const std::any& value) { return std::any_cast<std::string>(value);}
	static const char* getTypeName()
	{
		return "string";
	}

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
	static std::any getCatValue(std::unique_ptr<U>& value);
	static U* getValue(const std::any& value) {return static_cast<U*>(std::any_cast<Reflectable*>(value));}
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
	static std::any getCatValue(U* value);
	static U* getValue(const std::any& value) {return static_cast<U*>(std::any_cast<Reflectable*>(value));}
	static U* getPointer(U* value) {return value;};
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
	static std::any getCatValue(void) { return std::any();}
	static std::vector<ItemType>& getValue(const std::any& value) { *std::any_cast<std::vector<ItemType>*>(value)}

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
	static std::any getCatValue(void) { return std::any();}
	static std::map<std::string, ItemType>& getValue(const std::any& value) { return *std::any_cast<std::map<std::string, ItemType>*>(value);}

	typedef ItemType type;
	typedef std::map<std::string, ItemType> cachedType;
};


#include "TypeTraitsHeaderImplementation.h"
