/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"

#include <any>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace jitcat
{

	//These classes use template specialization to get properties of types relevant for the reflection, serialisation and expression system.
	//It allows to translate a type T to a CatGenericType and to check if a T is a reflectable/serialisable container.
	//The top class is the default case where T is neither a basic type nor a container type.
	//All other classes are specializations for specific types.
	template <typename T>
	class TypeTraits
	{
	public:
		static inline CatGenericType toGenericType();
		static bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static const char* getTypeName() { return T::getTypeName(); }
		static std::any getCatValue(void) { return std::any();}
		static std::any getCatValue(const T& value);
		static std::any getDefaultCatValue() { return std::any(); }
		static T* getValue(const std::any& value)  { return static_cast<T*>(std::any_cast<Reflection::Reflectable*>(value));}
		static Reflection::TypeInfo* getTypeInfo() {return Reflection::TypeRegistry::get()->registerType<T>();}

		typedef T type;
		typedef T cachedType;
		typedef T functionParameterType;
	};


	template <>
	class TypeTraits<void>
	{
	public:
		static CatGenericType toGenericType() { return CatGenericType::voidType; }
		static bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		template <typename U>
		static std::any getCatValue(const U& param) { return std::any();}
		static std::any getDefaultCatValue() { return std::any(); }

		static void getValue() { return;}
		static void getValue(const std::any& value) { return;}
		

		static const char* getTypeName()
		{
			return "void"; 
		}


		typedef void type;
		typedef int cachedType;
		typedef int functionParameterType;
	};


	template <>
	class TypeTraits<float>
	{
	public:
		static CatGenericType toGenericType() { return CatGenericType::floatType; }
		static bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static std::any getCatValue(float value) { return std::any(value);}
		static std::any getDefaultCatValue() { return std::any(0.0f); }
		static float getValue(const std::any& value) { return std::any_cast<float>(value);}
		static const char* getTypeName()
		{
			return "float"; 
		}


		typedef float type;
		typedef float cachedType;
		typedef float functionParameterType;
	};


	template <>
	class TypeTraits<int>
	{
	public:
		static CatGenericType toGenericType() { return CatGenericType::intType; }
		static bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static std::any getCatValue(int value) { return std::any(value);}
		static std::any getDefaultCatValue() { return std::any(0); }
		static int getValue(const std::any& value) { return std::any_cast<int>(value);}
		static const char* getTypeName()
		{
			return "int";
		}

		typedef int type;
		typedef int cachedType;
		typedef int functionParameterType;
	};


	template <>
	class TypeTraits<bool>
	{
	public:
		static CatGenericType toGenericType() { return CatGenericType::boolType; }
		static bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static std::any getCatValue(bool value) { return std::any(value);}
		static std::any getDefaultCatValue() { return std::any(false); }
		static bool getValue(const std::any& value) { return std::any_cast<bool>(value);}
		static const char* getTypeName()
		{
			return "bool";
		}

		typedef bool type;
		typedef bool cachedType;
		typedef bool functionParameterType;
	};


	template <>
	class TypeTraits<std::string>
	{
	public:
		static CatGenericType toGenericType() { return CatGenericType::stringType; }
		static bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static std::any getCatValue(const std::string& value) { return std::any(value);}
		static std::any getDefaultCatValue() { return std::any(""); }
		static std::string getValue(const std::any& value) { return std::any_cast<std::string>(value);}
		static const char* getTypeName()
		{
			return "string";
		}

		typedef std::string type;
		typedef std::string cachedType;
		typedef const std::string& functionParameterType;
	};


	template <typename U>
	class TypeTraits<std::unique_ptr<U>>
	{
	public:
		static CatGenericType toGenericType();
		static bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static const char* getTypeName() { return U::getTypeName(); }
		static std::any getCatValue(std::unique_ptr<U>& value);
		static std::any getDefaultCatValue() { return std::any((Reflection::Reflectable*)(nullptr)); }
		static U* getValue(const std::any& value) {return static_cast<U*>(std::any_cast<Reflection::Reflectable*>(value));}
		static Reflection::Reflectable* getPointer(std::unique_ptr<U>& value);
		static Reflection::TypeInfo* getTypeInfo() {return Reflection::TypeRegistry::get()->registerType<U>();}

		typedef U type;
		typedef U* cachedType;
		typedef U* functionParameterType;
	};


	template <typename U>
	class TypeTraits<U*>
	{
	public:
		static CatGenericType toGenericType();
		static bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static const char* getTypeName() { return U::getTypeName(); }
		static std::any getCatValue(U* value);
		static std::any getDefaultCatValue() { return std::any((Reflection::Reflectable*)(nullptr)); }
		static U* getValue(const std::any& value) {return static_cast<U*>(std::any_cast<Reflection::Reflectable*>(value));}
		static Reflection::Reflectable* getPointer(U* value) {return static_cast<Reflection::Reflectable*>(value);};
		static Reflection::TypeInfo* getTypeInfo() {return Reflection::TypeRegistry::get()->registerType<U>();}

		typedef U type;
		typedef U* cachedType;
		typedef U* functionParameterType;
	};


	template <typename ItemType>
	class TypeTraits<std::vector<ItemType> >
	{
	public:
		static CatGenericType toGenericType();
		static bool isSerialisableContainer() { return true; }
		static constexpr bool isReflectableType() { return false; }
		static const char* getTypeName() { return ""; }
		static std::any getCatValue(void) { return std::any();}
		static std::any getDefaultCatValue() { return std::any((std::vector<ItemType>*)(nullptr)); }
		static std::vector<ItemType>& getValue(const std::any& value) { *std::any_cast<std::vector<ItemType>*>(value); }

		typedef ItemType type;
		typedef std::vector<ItemType> cachedType;
		typedef std::vector<ItemType>* functionParameterType;
	};


	template <typename ItemType, typename ComparatorT>
	class TypeTraits<std::map<std::string, ItemType, ComparatorT> >
	{
	public:
		static CatGenericType toGenericType();
		static bool isSerialisableContainer() { return true; }
		static constexpr bool isReflectableType() { return false; }
		static const char* getTypeName() { return ""; }
		static std::any getCatValue(void) { return std::any();}
		static std::any getDefaultCatValue() { return std::any((std::map<std::string, ItemType, ComparatorT>*)(nullptr)); }
		static std::map<std::string, ItemType, ComparatorT>& getValue(const std::any& value) { return *std::any_cast<std::map<std::string, ItemType>*>(value);}

		typedef ItemType type;
		typedef std::map<std::string, ItemType, ComparatorT> cachedType;
		typedef std::map<std::string, ItemType, ComparatorT>* functionParameterType;
	};

} //End namespace jitcat

#include "jitcat/TypeTraitsHeaderImplementation.h"