/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/TypeOwnershipSemantics.h"
#include "jitcat/STLTypeReflectors.h"
#include "jitcat/TypeRegistry.h"
#include "jitcat/TypeTools.h"

namespace jitcat
{
	template<typename ObjectT, typename EnabledT>
	const CatGenericType& TypeTraits<ObjectT, EnabledT>::toGenericType()
	{
		static_assert(std::is_class_v<ObjectT>, "Type is not supported.");
		Reflection::TypeInfo* nullTypeInfo = nullptr;
		static std::unique_ptr<CatGenericType> type(std::make_unique<CatGenericType>(nullTypeInfo));
		Reflection::TypeRegistry::get()->registerType<ObjectT>(type->getTypeInfoToSet());
		return *type.get();
	}


	template<typename ObjectT>
	const CatGenericType& TypeTraits<ObjectT, std::enable_if_t<std::is_abstract_v<ObjectT>>>::toGenericType()
	{
		static_assert(std::is_class_v<ObjectT>, "Type is not supported.");
		Reflection::TypeInfo* nullTypeInfo = nullptr;
		static std::unique_ptr<CatGenericType> type(std::make_unique<CatGenericType>(nullTypeInfo));
		Reflection::TypeInfo* typeInfo = Reflection::TypeRegistry::get()->registerType<ObjectT>(type->getTypeInfoToSet());
		(void)typeInfo;
		return *type.get();
	}


	template<typename ObjectT>
	const char* TypeTraits<ObjectT, std::enable_if_t<std::is_abstract_v<ObjectT>>>::getTypeName()
	{ 
		return Reflection::TypeNameGetter<ObjectT>::get();
	}


	template<typename EnumT>
	const CatGenericType& TypeTraits<EnumT, std::enable_if_t<std::is_enum_v<EnumT>>>::toGenericType()
	{
		Reflection::TypeInfo* enumInfo = Reflection::TypeRegistry::get()->registerType<EnumT>();
		static std::unique_ptr<CatGenericType> enumType = std::make_unique<CatGenericType>(TypeTraits<typename std::underlying_type_t<EnumT>>::toGenericType(), enumInfo);
		return *enumType.get();
	}


	template<typename ObjectT, typename EnabledT>
	std::any TypeTraits<ObjectT, EnabledT>::getCatValue(ObjectT&& value)
	{
		return std::move(value);
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


		template<typename ObjectT>
	std::any TypeTraits<ObjectT, std::enable_if_t<std::is_abstract_v<ObjectT>>>::getCatValue(ObjectT&& value)
	{
		return std::move(value);
	}


	template<typename ObjectT>
	inline std::any TypeTraits<ObjectT, std::enable_if_t<std::is_abstract_v<ObjectT>>>::getCatValue(ObjectT& value)
	{
		return value;
	}

	
	template <typename PointerT>
	inline const CatGenericType& TypeTraits<const PointerT*>::toGenericType()
	{
		return TypeTraits<PointerT*>::toGenericType();
	}


	template <typename PointerT>
	const CatGenericType& TypeTraits<PointerT*>::toGenericType()
	{
		static std::unique_ptr<CatGenericType> type;
		if (type.get() == nullptr)
		{
			//Instead directly constructing the static type object. First construct it with a nullptr for the object type.
			//This is done to prevent recursion deadlock.
			CatGenericType pointeeType = TypeTraits<PointerT>::toGenericType();
			if (type.get() == nullptr)
			{
				type = std::make_unique<CatGenericType>(pointeeType, Reflection::TypeOwnershipSemantics::Weak, false, false, false, false);
			}
		}
		return *type.get();
	}


	template <typename RefT>
	inline const CatGenericType& TypeTraits<const RefT&>::toGenericType()
	{
		static std::unique_ptr<CatGenericType> type;
		if (type == nullptr)
		{
			CatGenericType pointeeType = TypeTraits<RefT>::toGenericType();
			if (type.get() == nullptr)
			{
				type = std::make_unique<CatGenericType>(pointeeType, Reflection::TypeOwnershipSemantics::Weak, false, false, false, true);
			}
		}
		return *type.get();
	}
	

	template <typename RefT>
	inline const CatGenericType& TypeTraits<RefT&>::toGenericType()
	{
		static std::unique_ptr<CatGenericType> type;
		if (type == nullptr)
		{
			CatGenericType pointeeType = TypeTraits<RefT>::toGenericType();
			if (type.get() == nullptr)
			{
				type = std::make_unique<CatGenericType>(pointeeType, Reflection::TypeOwnershipSemantics::Weak, false, true, false, true);
			}
		}
		return *type.get();
	}


	template<typename PointerRefT>
	const CatGenericType& TypeTraits<PointerRefT*&>::toGenericType()
	{
		static std::unique_ptr<CatGenericType> type;
		if (type == nullptr)
		{
			CatGenericType pointeeType = TypeTraits<PointerRefT*>::toGenericType();
			if (type.get() == nullptr)
			{
				type = std::make_unique<CatGenericType>(pointeeType, Reflection::TypeOwnershipSemantics::Weak, false, true, false, true);
			}
		}
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


	template <typename FundamentalT>
	const char* TypeTraits<FundamentalT, std::enable_if_t<(std::is_fundamental_v<FundamentalT> && !std::is_void_v<FundamentalT>) || std::is_same_v<FundamentalT, std::array<float, 4>> > >::getTypeName()
	{
		return Reflection::TypeNameGetter<FundamentalT>::get();
	}


	template <typename EnumT>
	const char* TypeTraits<EnumT, std::enable_if_t<std::is_enum_v<EnumT>>>::getTypeName()
	{
		return Reflection::getEnumName<EnumT>();	
	}
}