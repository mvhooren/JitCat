/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatHostClass.h"
#include <functional>


namespace jitcat
{
	namespace Reflection
	{
		class Reflectable;
	}
	template<typename ReflectableT>
	class CatDeferredHostClass : public CatHostClass
	{
	public:
		CatDeferredHostClass(bool constructible, bool placementConstructible, bool inheritable,
							 std::function<Reflection::Reflectable* ()>& constructor,
							 std::function<void (Reflection::Reflectable*)>& destructor,
							 std::function<void (unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
							 std::function<void (unsigned char* buffer, std::size_t bufferSize)>& placementDestructor):
			CatHostClass(constructible, placementConstructible, inheritable),
			constructor(constructor),
			destructor(destructor),
			placementConstructor(placementConstructor),
			placementDestructor(placementDestructor)
		{}


		virtual Reflection::Reflectable* construct() override final
		{
			return constructor();
		}


		virtual void destruct(Reflection::Reflectable* object) override final
		{
			destructor(object);
		}


		virtual void placementConstruct(unsigned char* data, std::size_t dataSize) override final
		{
			placementConstructor(data, dataSize);
		}


		virtual void placementDestruct(unsigned char* data, std::size_t dataSize) override final
		{
			placementDestructor(data, dataSize);
		}

	private:
		std::function<Reflection::Reflectable* ()> constructor;
		std::function<void(Reflection::Reflectable*)> destructor;
		std::function<void (unsigned char* buffer, std::size_t bufferSize)> placementConstructor;
		std::function<void (unsigned char* buffer, std::size_t bufferSize)> placementDestructor;
	};

}