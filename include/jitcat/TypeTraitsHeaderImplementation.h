/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "ContainerMemberReference.h"
#include "ObjectMemberReference.h"
#include "TypeRegistry.h"

template<typename T>
CatGenericType TypeTraits<T>::toGenericType()
{
	return CatGenericType(TypeRegistry::get()->registerType<T>()); 
}


template<typename T>
inline T* TypeTraits<T>::getValueFromMemberReference(MemberReference* value)
{
	if (value != nullptr
		&& value->getCatType() == CatType::Object
		&& value->getCustomTypeName() == T::getTypeName())
	{
		return static_cast<ObjectMemberReference<T>*>(value)->getObject();
	}
	return nullptr;
}


template<typename T>
inline CatValue TypeTraits<T>::getCatValue(const T& value)
{
	MemberReferencePtr ptr(new ObjectMemberReference<T>(&value, nullptr, TypeRegistry::get()->registerType<T>()));
	return CatValue(ptr);
}


template <typename U>
CatValue TypeTraits<U*>::getCatValue(U* value) 
{ 
	MemberReferencePtr ptr(new ObjectMemberReference<U>(value, nullptr, TypeRegistry::get()->registerType<U>()));
	return CatValue(ptr);
}


template <typename U>
CatGenericType TypeTraits<U*>::toGenericType() 
{ 
	return CatGenericType(TypeRegistry::get()->registerType<U>()); 
}


template <typename U>
inline U* TypeTraits<U*>::getValueFromMemberReference(MemberReference* value)
{
	if (value != nullptr
		&& value->getCatType() == CatType::Object
		&& value->getCustomTypeName() == U::getTypeName())
	{
		return static_cast<ObjectMemberReference<U>*>(value)->getObject();
	}
	return nullptr;
}


template <typename U>
CatValue TypeTraits<std::unique_ptr<U>>::getCatValue(std::unique_ptr<U>& value) { return CatValue(ObjectMemberReference<U*>(value.get(), nullptr, TypeRegistry::get()->registerType<U>()));}
template<typename U>

inline U* TypeTraits<std::unique_ptr<U>>::getPointer(std::unique_ptr<U>& value)
{
	return value.get();
}

template <typename U>
CatGenericType TypeTraits<std::unique_ptr<U>>::toGenericType() { return CatGenericType(TypeRegistry::get()->registerType<U>()); }


template <typename U>
inline U* TypeTraits<std::unique_ptr<U>>::getValueFromMemberReference(MemberReference* value)
{
	if (value != nullptr
		&& value->getCatType() == CatType::Object
		&& value->getCustomTypeName() == U::getTypeName())
	{
		return static_cast<ObjectMemberReference<U>*>(value)->getObject();
	}
	return nullptr;
}


template <typename ItemType>
inline std::vector<ItemType>& TypeTraits<std::vector<ItemType> >::getValueFromMemberReference(MemberReference* value)
{
	if (value != nullptr
		&& value->getCatType() == CatType::Object
		&& value->getContainerType() == ContainerType::Vector
		&& value->getCustomTypeName() == TypeTraits<ItemType>::getTypeName())
	{
		ContainerMemberReference<std::vector<ItemType> >* containerReference = static_cast<ContainerMemberReference<std::vector<ItemType> >*>(value);
		if (containerReference->parentObject.getIsValid())
		{
			return containerReference->container;
		}
	}
	static std::vector<ItemType> emptyVector = std::vector<ItemType>();
	return emptyVector;
}


template <typename ItemType>
std::map<std::string, ItemType>& TypeTraits<std::map<std::string, ItemType> >::getValueFromMemberReference(MemberReference* value)
{
	if (value != nullptr
		&& value->getCatType() == CatType::Object
		&& value->getContainerType() == ContainerType::StringMap
		&& value->getCustomTypeName() == TypeTraits<ItemType>::getTypeName())
	{
		ContainerMemberReference<std::map<std::string, ItemType> >* containerReference = static_cast<ContainerMemberReference<std::map<std::string, ItemType> >*>(value);
		if (containerReference->parentObject.getIsValid())
		{
			return containerReference->container;
		}
	}
	static std::map<std::string, ItemType> emptyMap = std::map<std::string, ItemType>();
	return emptyMap;
}


template <typename ItemType>
CatGenericType TypeTraits<std::vector<ItemType>>::toGenericType() { return CatGenericType(ContainerType::Vector, TypeRegistry::get()->registerType<ItemType>()); }
template <typename ItemType>
CatGenericType TypeTraits<std::map<std::string, ItemType>>::toGenericType() { return CatGenericType(ContainerType::StringMap, TypeTraits<ItemType>::getTypeInfo()); }
