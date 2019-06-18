/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ReflectableHandle.h"

namespace jitcat::Reflection
{
	class Reflectable;
	class TypeInfo;

	class ReflectableInstance
	{
	public:
		ReflectableInstance();
		ReflectableInstance(Reflectable* reflectable, TypeInfo* reflectableType);
		~ReflectableInstance();
		ReflectableInstance& operator=(ReflectableInstance&& other) noexcept;

		Reflectable* getReflectable() const;
		TypeInfo* getType() const;

	private:
		ReflectableHandle reflectable;
		TypeInfo* reflectableType;
	};
}