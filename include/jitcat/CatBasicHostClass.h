/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatHostClass.h"

#include <cassert>

namespace jitcat
{
	template<typename ReflectableT>
	class CatBasicHostClass : public CatHostClass
	{
	public:
		CatBasicHostClass(bool constructible, bool placementConstructible, bool inheritable) : CatHostClass(constructible, placementConstructible, inheritable) {};


		virtual Reflection::Reflectable* construct() override final
		{
			return new ReflectableT();
		}


		virtual void destruct(Reflection::Reflectable* object) override final
		{
			delete (ReflectableT*)object;
		}


		virtual void placementConstruct(unsigned char* data, std::size_t dataSize) override final
		{
			assert(sizeof(ReflectableT) >= dataSize);
			new(data) ReflectableT();
		}


		virtual void placementDestruct(unsigned char* data, std::size_t dataSize) override final
		{
			assert(sizeof(ReflectableT) >= dataSize);
			reinterpret_cast<ReflectableT*>(data)->~ReflectableT();
		}
	};

}