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
		CatDeferredHostClass(bool constructible, bool inheritable,
							 std::function<Reflection::Reflectable* ()>& constructor,
							 std::function<void (Reflection::Reflectable*)>& destructor) :
			CatHostClass(constructible, inheritable),
			constructor(constructor),
			destructor(destructor)
		{}
		virtual Reflection::Reflectable* construct() override final
		{
			return constructor();
		}
		virtual void destruct(Reflection::Reflectable* object) override final
		{
			destructor(object);
		}

	private:
		std::function<Reflection::Reflectable* ()> constructor;
		std::function<void(Reflection::Reflectable*)> destructor;
	};

}