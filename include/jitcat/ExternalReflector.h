/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once

#include <type_traits>

namespace jitcat::Reflection
{
	class ReflectedEnumTypeInfo;
	class ReflectedTypeInfo;


	template <typename ReflectableT>
	class ExternalReflector
	{
	private:
		ExternalReflector() = delete;
		~ExternalReflector() = delete;
		ExternalReflector(const ExternalReflector&) = delete;
		
	public:
		static const char* getTypeName();
		static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);

		static constexpr bool exists = false;
	};


	template <typename EnumT>
	const char* getEnumName()
	{
		static_assert(!std::is_enum_v<EnumT>, "This function needs to be implemented for this enum.");
		return nullptr;
	}

	template <typename EnumT>
	void reflectEnum(ReflectedEnumTypeInfo& enumTypeInfo)
	{
		static_assert(!std::is_enum_v<EnumT>, "This function needs to be implemented for this enum.");
	}

	template <typename T, typename = int>
	struct CopyConstructControlVariableExists : std::false_type { };

	template <typename T>
	struct CopyConstructControlVariableExists <T, decltype((void) T::enableCopyConstruction, 0)> : std::true_type { };

	namespace TypeTools
	{
	template<typename ReflectableT>
	inline constexpr bool getAllowCopyConstruction()
	{
		if constexpr (CopyConstructControlVariableExists<ReflectableT>::value)
		{
			return ReflectableT::enableCopyConstruction;
		}
		else if constexpr (ExternalReflector<ReflectableT>::exists)
		{
			if constexpr (CopyConstructControlVariableExists<ExternalReflector<ReflectableT>>::value)
			{
				return ExternalReflector<ReflectableT>::enableCopyConstruction;
			}
			else 
			{
				return std::is_copy_constructible<ReflectableT>::value;
			}
		}
		else
		{
			return std::is_copy_constructible<ReflectableT>::value;
		}
	}
	}
}