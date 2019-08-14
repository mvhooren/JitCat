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
		static inline const CatGenericType& toGenericType();

		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return T::getTypeName(); }
		static std::any getCatValue(void) { return std::any((Reflectable*)nullptr);}
		static std::any getCatValue(T& value);
		static constexpr Reflection::Reflectable* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<T>::getDefaultValue()); }
		static T* getValue(const std::any& value)  { return static_cast<T*>(std::any_cast<Reflection::Reflectable*>(value));}
		static Reflection::TypeInfo* getTypeInfo() {return Reflection::TypeRegistry::get()->registerType<T>();}
		static Reflection::Reflectable* stripValue(T& value) { return static_cast<Reflection::Reflectable*>(&value); }
		static Reflection::Reflectable* stripValue(T* value) {return static_cast<Reflection::Reflectable*>(value);}
		
		typedef T* getValueType;
		typedef T type;
		typedef T cachedType;
		typedef T functionParameterType;
		typedef Reflection::Reflectable* functionReturnType;
	};


	template <>
	class TypeTraits<void>
	{
	public:
		static const CatGenericType& toGenericType() { return CatGenericType::voidType; }
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		template <typename U>
		static std::any getCatValue(const U& param) { return std::any();}
		static constexpr void getDefaultValue() { return; }
		static std::any getDefaultCatValue() { return std::any(); }
		static void stripValue(void) { }
		static void getValue() { return;}
		static void getValue(const std::any& value) { return;}
		

		static const char* getTypeName()
		{
			return "void"; 
		}

		typedef void getValueType;
		typedef void type;
		typedef int cachedType;
		typedef int functionParameterType;
		typedef int functionReturnType;
	};


	template <>
	class TypeTraits<float>
	{
	public:
		static const CatGenericType& toGenericType() { return CatGenericType::floatType; }
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static std::any getCatValue(float value) { return std::any(value);}
		static constexpr float getDefaultValue() { return 0.0f; }
		static std::any getDefaultCatValue() { return std::any(0.0f); }
		static float getValue(const std::any& value) { return std::any_cast<float>(value);}
		static float stripValue(float value) { return value; }
		static const char* getTypeName()
		{
			return "float"; 
		}

		typedef float getValueType;
		typedef float type;
		typedef float cachedType;
		typedef float functionParameterType;
		typedef float functionReturnType;
	};


	template <>
	class TypeTraits<int>
	{
	public:
		static const CatGenericType& toGenericType() { return CatGenericType::intType; }
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static std::any getCatValue(int value) { return std::any(value);}
		static constexpr int getDefaultValue() { return 0; }
		static std::any getDefaultCatValue() { return std::any(0); }
		static int getValue(const std::any& value) { return std::any_cast<int>(value);}
		static int stripValue(int value) { return value; }
		static const char* getTypeName()
		{
			return "int";
		}

		typedef int getValueType;
		typedef int type;
		typedef int cachedType;
		typedef int functionParameterType;
		typedef int functionReturnType;
	};


	template <>
	class TypeTraits<bool>
	{
	public:
		static const CatGenericType& toGenericType() { return CatGenericType::boolType; }
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static std::any getCatValue(bool value) { return std::any(value);}
		static constexpr bool getDefaultValue() { return false; }
		static std::any getDefaultCatValue() { return std::any(false); }
		static bool getValue(const std::any& value) { return std::any_cast<bool>(value);}
		
		static bool stripValue(bool value) { return value; }
		static const char* getTypeName()
		{
			return "bool";
		}

		typedef bool getValueType;
		typedef bool type;
		typedef bool cachedType;
		typedef bool functionParameterType;
		typedef bool functionReturnType;
	};


	template <>
	class TypeTraits<std::string>
	{
	public:
		static const CatGenericType& toGenericType() { return CatGenericType::stringType; }
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static std::any getCatValue(const std::string& value) { return std::any(value);}
		static std::string getDefaultValue() { return Tools::empty; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<std::string>::getDefaultValue()); }
		static std::string getValue(const std::any& value) { return std::any_cast<std::string>(value);}
		static const std::string& stripValue(const std::string& value) { return value; }
		static const char* getTypeName()
		{
			return "string";
		}

		typedef std::string getValueType;
		typedef std::string type;
		typedef std::string cachedType;
		typedef const std::string& functionParameterType;
		typedef std::string functionReturnType;
	};


	template <typename U>
	class TypeTraits<std::unique_ptr<U>>
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return true; }

		static const char* getTypeName() { return U::getTypeName(); }
		static std::any getCatValue(std::unique_ptr<U>& value);
		static constexpr Reflection::Reflectable* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<std::unique_ptr<U>>::getDefaultValue()); }
		static U* getValue(const std::any& value) {return static_cast<U*>(std::any_cast<Reflection::Reflectable*>(value));}
		static Reflection::Reflectable* stripValue(std::unique_ptr<U>& value) { return static_cast<Reflection::Reflectable*>(value.get()); }
		static Reflection::TypeInfo* getTypeInfo() {return Reflection::TypeRegistry::get()->registerType<U>();}

		typedef U* getValueType;
		typedef U type;
		typedef U* cachedType;
		typedef U* functionParameterType;
		typedef Reflection::Reflectable* functionReturnType;
	};


	template <typename U>
	class TypeTraits<U*>
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isSerialisableContainer() { return false; }
		static constexpr bool isReflectableType() { return true; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return U::getTypeName(); }
		static std::any getCatValue(U* value);
		static constexpr Reflection::Reflectable* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<U*>::getDefaultValue()); }
		static U* getValue(const std::any& value) {return static_cast<U*>(std::any_cast<Reflection::Reflectable*>(value));}
		static Reflection::Reflectable* stripValue(U* value) {return static_cast<Reflection::Reflectable*>(value);}
		static Reflection::TypeInfo* getTypeInfo() {return Reflection::TypeRegistry::get()->registerType<U>();}

		typedef U* getValueType;
		typedef U type;
		typedef U* cachedType;
		typedef U* functionParameterType;
		typedef Reflection::Reflectable* functionReturnType;
	};


	template <typename ItemType, typename AllocatorT>
	class TypeTraits<std::vector<ItemType, AllocatorT> >
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isSerialisableContainer() { return true; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return ""; }
		static std::any getCatValue(void) { return std::any((std::vector<ItemType, AllocatorT>*)nullptr);}
		static constexpr std::vector<ItemType, AllocatorT>* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<std::vector<ItemType, AllocatorT> >::getDefaultValue()); }
		static std::vector<ItemType, AllocatorT>& getValue(const std::any& value) { *std::any_cast<std::vector<ItemType, AllocatorT>*>(value); }
		static std::vector<ItemType, AllocatorT>* stripValue(std::vector<ItemType, AllocatorT>* value) { return value; }

		typedef std::vector<ItemType, AllocatorT>& getValueType;
		typedef ItemType type;
		typedef std::vector<ItemType, AllocatorT> cachedType;
		typedef std::vector<ItemType, AllocatorT>* functionParameterType;
		typedef std::vector<ItemType, AllocatorT>* functionReturnType;
	};


	template <typename KeyType, typename ItemType, typename ComparatorT, typename AllocatorT>
	class TypeTraits<std::map<KeyType, ItemType, ComparatorT, AllocatorT> >
	{
	public:
		static const CatGenericType& toGenericType();
		static constexpr bool isSerialisableContainer() { return true; }
		static constexpr bool isReflectableType() { return false; }
		static constexpr bool isUniquePtr() { return false; }

		static const char* getTypeName() { return ""; }
		static std::any getCatValue(void) { return std::any((std::map<KeyType, ItemType, ComparatorT, AllocatorT>*)nullptr);}
		static constexpr std::map<KeyType, ItemType, ComparatorT, AllocatorT>* getDefaultValue() { return nullptr; }
		static std::any getDefaultCatValue() { return std::any(TypeTraits<std::map<KeyType, ItemType, ComparatorT, AllocatorT> >::getDefaultValue()); }
		static std::map<KeyType, ItemType, ComparatorT, AllocatorT>& getValue(const std::any& value) { return *std::any_cast<std::map<KeyType, ItemType, ComparatorT, AllocatorT>*>(value);}
		static std::map<KeyType, ItemType, ComparatorT, AllocatorT>* stripValue(std::map<KeyType, ItemType, ComparatorT, AllocatorT>* value) { return value; }

		typedef std::map<KeyType, ItemType, ComparatorT, AllocatorT>& getValueType;
		typedef ItemType type;
		typedef std::map<KeyType, ItemType, ComparatorT, AllocatorT> cachedType;
		typedef std::map<KeyType, ItemType, ComparatorT, AllocatorT>* functionParameterType;
		typedef std::map<KeyType, ItemType, ComparatorT, AllocatorT>* functionReturnType;
	};

} //End namespace jitcat

#include "jitcat/TypeTraitsHeaderImplementation.h"