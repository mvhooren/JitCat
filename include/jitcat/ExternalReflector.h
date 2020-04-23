/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once

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
		static_assert(false, "This function needs to be implemented for this enum.");
		return nullptr;
	}

	template <typename EnumT>
	void reflectEnum(ReflectedEnumTypeInfo& enumTypeInfo)
	{
		static_assert(false, "This function needs to be implemented for this enum.");
	}
}