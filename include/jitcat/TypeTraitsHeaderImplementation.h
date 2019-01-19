/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "TypeRegistry.h"

template<typename T>
CatGenericType TypeTraits<T>::toGenericType()
{
	return CatGenericType(TypeRegistry::get()->registerType<T>()); 
}


template<typename T>
inline std::any TypeTraits<T>::getCatValue(const T& value)
{
	return &value;
}


template <typename U>
std::any TypeTraits<U*>::getCatValue(U* value) 
{ 
	return static_cast<Reflectable*>(value);
}


template <typename U>
CatGenericType TypeTraits<U*>::toGenericType() 
{ 
	return CatGenericType(TypeRegistry::get()->registerType<U>()); 
}


template <typename U>
std::any TypeTraits<std::unique_ptr<U>>::getCatValue(std::unique_ptr<U>& value) { return static_cast<Reflectable*>(value.get());}

template<typename U>
inline U* TypeTraits<std::unique_ptr<U>>::getPointer(std::unique_ptr<U>& value)
{
	return static_cast<Reflectable*>(value.get());
}

template <typename U>
CatGenericType TypeTraits<std::unique_ptr<U>>::toGenericType() { return CatGenericType(TypeRegistry::get()->registerType<U>()); }


template <typename ItemType>
CatGenericType TypeTraits<std::vector<ItemType>>::toGenericType() { return CatGenericType(ContainerType::Vector, TypeRegistry::get()->registerType<ItemType>()); }

template <typename ItemType>
CatGenericType TypeTraits<std::map<std::string, ItemType>>::toGenericType() { return CatGenericType(ContainerType::StringMap, TypeTraits<ItemType>::getTypeInfo()); }
