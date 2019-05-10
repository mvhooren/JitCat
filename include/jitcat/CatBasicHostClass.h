/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatHostClass.h"

namespace jitcat
{
	template<typename ReflectableT>
	class CatBasicHostClass : public CatHostClass
	{
	public:
		CatBasicHostClass(bool constructible, bool inheritable) : CatHostClass(constructible, inheritable) {};
		virtual Reflection::Reflectable* construct() override final
		{
			return new ReflectableT();
		}
		virtual void destruct(Reflection::Reflectable* object) override final
		{
			delete object;
		}

	};

}