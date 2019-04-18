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
	CatGenericType TypeTraits<T>::toGenericType()
	{
		return CatGenericType(Reflection::TypeRegistry::get()->registerType<T>());
	}


	template<typename T>
	inline std::any TypeTraits<T>::getCatValue(const T& value)
	{
		return &value;
	}


	template <typename U>
	std::any TypeTraits<U*>::getCatValue(U* value)
	{
		return static_cast<Reflection::Reflectable*>(value);
	}


	template <typename U>
	CatGenericType TypeTraits<U*>::toGenericType()
	{
		return CatGenericType(Reflection::TypeRegistry::get()->registerType<U>());
	}


	template <typename U>
	std::any TypeTraits<std::unique_ptr<U>>::getCatValue(std::unique_ptr<U>& value) { return static_cast<Reflection::Reflectable*>(value.get()); }

	template<typename U>
	inline Reflection::Reflectable* TypeTraits<std::unique_ptr<U>>::getPointer(std::unique_ptr<U>& value)
	{
		return static_cast<Reflection::Reflectable*>(value.get());
	}

	template <typename U>
	CatGenericType TypeTraits<std::unique_ptr<U>>::toGenericType() { return CatGenericType(Reflection::TypeRegistry::get()->registerType<U>()); }


	template <typename ItemType>
	CatGenericType TypeTraits<std::vector<ItemType>>::toGenericType()
	{
		static std::unique_ptr<Reflection::ContainerManipulator> vectorManipulator(new jitcat::Reflection::VectorManipulator<std::vector<ItemType>>());
		return CatGenericType(Reflection::ContainerType::Vector, vectorManipulator.get(), Reflection::TypeRegistry::get()->registerType<typename TypeTraits<ItemType>::type>());
	}

	template <typename ItemType, typename ComparatorT>
	CatGenericType TypeTraits<std::map<std::string, ItemType, ComparatorT>>::toGenericType()
	{
		static std::unique_ptr<Reflection::ContainerManipulator> mapManipulator(new jitcat::Reflection::MapManipulator<std::map<std::string, ItemType, ComparatorT>>());
		return CatGenericType(Reflection::ContainerType::StringMap, mapManipulator.get(), TypeTraits<typename TypeTraits<ItemType>::type>::getTypeInfo());
	}

}