/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "ContainerManipulator.h"


namespace jitcat
{

	template<typename T>
	const CatGenericType& TypeTraits<T>::toGenericType()
	{
		TypeInfo* typeInfo = Reflection::TypeRegistry::get()->registerType<T>();
		static std::unique_ptr<CatGenericType> type(new CatGenericType(typeInfo));
		return *type.get();
	}


	template<typename T>
	inline std::any TypeTraits<T>::getCatValue(T& value)
	{
		return static_cast<Reflection::Reflectable*>(&value);
	}


	template <typename U>
	std::any TypeTraits<U*>::getCatValue(U* value)
	{
		return static_cast<Reflection::Reflectable*>(value);
	}


	template <typename U>
	const CatGenericType& TypeTraits<U*>::toGenericType()
	{
		TypeInfo* typeInfo = Reflection::TypeRegistry::get()->registerType<U>();
		static std::unique_ptr<CatGenericType> type(new CatGenericType(typeInfo));
		return *type.get();
	}


	template <typename U>
	std::any TypeTraits<std::unique_ptr<U>>::getCatValue(std::unique_ptr<U>& value) 
	{ 
		return static_cast<Reflection::Reflectable*>(value.get()); 
	}


	template <typename U>
	const CatGenericType& TypeTraits<std::unique_ptr<U>>::toGenericType() 
	{
		return TypeTraits<U*>::toGenericType();
	}


	template <typename ItemType, typename AllocatorT>
	const CatGenericType& TypeTraits<std::vector<ItemType, AllocatorT>>::toGenericType()
	{
		//Make sure that the item type is known to the type system.
		TypeTraits<ItemType>::toGenericType();
		static std::unique_ptr<Reflection::ContainerManipulator> vectorManipulator(new jitcat::Reflection::VectorManipulator<std::vector<ItemType, AllocatorT>>());
		static std::unique_ptr<CatGenericType> type(new CatGenericType(Reflection::ContainerType::Vector, vectorManipulator.get()));
		return *type.get();
	}


	template <typename KeyType, typename ItemType, typename ComparatorT, typename AllocatorT>
	const CatGenericType& TypeTraits<std::map<KeyType, ItemType, ComparatorT, AllocatorT>>::toGenericType()
	{
		//Make sure that the key type and item type are known to the type system.
		TypeTraits<ItemType>::toGenericType();
		TypeTraits<KeyType>::toGenericType();
		static std::unique_ptr<Reflection::ContainerManipulator> mapManipulator(new jitcat::Reflection::MapManipulator<std::map<KeyType, ItemType, ComparatorT, AllocatorT>>());
		static std::unique_ptr<CatGenericType> type(new CatGenericType(Reflection::ContainerType::Map, mapManipulator.get()));
		return *type.get();
	}

}