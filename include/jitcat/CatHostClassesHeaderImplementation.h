#pragma once

#include "jitcat/CatBasicHostClass.h"
#include "jitcat/CatDeferredHostClass.h"
#include "jitcat/Reflectable.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"

namespace jitcat
{
	template<typename ReflectableT>
	inline void CatHostClasses::addHostClass(bool constructible, bool inheritable)
	{
		Reflection::TypeRegistry::get()->registerType<ReflectableT>();
		static_assert(std::is_base_of<Reflection::Reflectable, ReflectableT>::value, "ReflectableT should inherit from Reflectable.");
		static_assert(std::is_default_constructible<ReflectableT>::value, "ReflectableT must be default constructible. Non-default constructible classes should provide construction lambdas.");
		std::string lowerCaseTypeName = Tools::toLowerCase(ReflectableT::getTypeName());
		if (hostClasses.find(lowerCaseTypeName) == hostClasses.end())
		{
			hostClasses.emplace(lowerCaseTypeName, new CatBasicHostClass<ReflectableT>(constructible, inheritable));
		}
	}

	template<typename ReflectableT>
	inline void CatHostClasses::addHostClass(bool constructible, bool inheritable,
											std::function<ReflectableT* ()> constructor,
											std::function<void(ReflectableT*)> destructor)
	{
		Reflection::TypeRegistry::get()->registerType<ReflectableT>();
		static_assert(std::is_base_of<Reflection::Reflectable, ReflectableT>::value, "ReflectableT should inherit from Reflectable.");
		std::string lowerCaseTypeName = Tools::toLowerCase(ReflectableT::getTypeName());
		if (hostClasses.find(lowerCaseTypeName) = hostClasses.end())
		{
			hostClasses.emplace(lowerCaseTypeName, new CatDeferredHostClass<ReflectableT>(constructible, inheritable, constructor, destructor));
		}
	}
}