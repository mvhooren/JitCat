/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ContainerManipulator.h"
#include "jitcat/TypeOwnershipSemantics.h"


namespace jitcat
{
	template<typename ObjectT, typename EnabledT>
	const CatGenericType& TypeTraits<ObjectT, EnabledT>::toGenericType()
	{
		static_assert(std::is_class_v<ObjectT>, "Type is not supported.");
		Reflection::TypeInfo* typeInfo = Reflection::TypeRegistry::get()->registerType<ObjectT>();
		static std::unique_ptr<CatGenericType> type(std::make_unique<CatGenericType>(typeInfo));
		return *type.get();
	}


	template<typename ObjectT, typename EnabledT>
	inline std::any TypeTraits<ObjectT, EnabledT>::getCatValue(ObjectT& value)
	{
		return value;
	}

	
	template <typename PointerT>
	std::any TypeTraits<PointerT*>::getCatValue(PointerT* value)
	{
		return value;
	}


	template <typename PointerT>
	const CatGenericType& TypeTraits<PointerT*>::toGenericType()
	{
		static std::unique_ptr<CatGenericType> type(std::make_unique<CatGenericType>(TypeTraits<PointerT>::toGenericType(), TypeOwnershipSemantics::Weak, false));
		return *type.get();
	}


	template <typename RefT>
	inline const CatGenericType& TypeTraits<RefT&>::toGenericType()
	{
		return TypeTraits<RefT*>::toGenericType();
	}

	template<typename PointerRefT>
	const CatGenericType& TypeTraits<PointerRefT*&>::toGenericType()
	{
		static std::unique_ptr<CatGenericType> type(std::make_unique<CatGenericType>(TypeTraits<PointerRefT*>::toGenericType(), TypeOwnershipSemantics::Weak, false));
		return *type.get();
	}

	   	 
	template <typename UniquePtrT>
	std::any TypeTraits<std::unique_ptr<UniquePtrT>>::getCatValue(std::unique_ptr<UniquePtrT>& value) 
	{ 
		return value.get(); 
	}


	template <typename UniquePtrT>
	const CatGenericType& TypeTraits<std::unique_ptr<UniquePtrT>>::toGenericType() 
	{
		return TypeTraits<UniquePtrT*>::toGenericType();
	}


	template <typename ItemType, typename AllocatorT>
	const CatGenericType& TypeTraits<std::vector<ItemType, AllocatorT>>::toGenericType()
	{
		//Make sure that the item type is known to the type system.
		TypeTraits<ItemType>::toGenericType();
		static std::unique_ptr<Reflection::ContainerManipulator> vectorManipulator(new jitcat::Reflection::VectorManipulator<std::vector<ItemType, AllocatorT>>());
		static std::unique_ptr<CatGenericType> type(std::make_unique<CatGenericType>(Reflection::ContainerType::Vector, vectorManipulator.get()));
		return *type.get();
	}


	template <typename KeyType, typename ItemType, typename ComparatorT, typename AllocatorT>
	const CatGenericType& TypeTraits<std::map<KeyType, ItemType, ComparatorT, AllocatorT>>::toGenericType()
	{
		//Make sure that the key type and item type are known to the type system.
		TypeTraits<ItemType>::toGenericType();
		TypeTraits<KeyType>::toGenericType();
		static std::unique_ptr<Reflection::ContainerManipulator> mapManipulator(new jitcat::Reflection::MapManipulator<std::map<KeyType, ItemType, ComparatorT, AllocatorT>>());
		static std::unique_ptr<CatGenericType> type(std::make_unique<CatGenericType>(Reflection::ContainerType::Map, mapManipulator.get()));
		return *type.get();
	}

}