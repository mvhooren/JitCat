/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ReflectableHandle.h"

#include <any>


namespace jitcat::Reflection
{
	class Reflectable;
	class TypeInfo;

	class ObjectInstance
	{
	public:
		ObjectInstance();
		ObjectInstance(unsigned char* object, TypeInfo* objectType);
		~ObjectInstance();
		ObjectInstance& operator=(ObjectInstance&& other) noexcept;

		unsigned char* getObject() const;
		TypeInfo* getType() const;
		std::any getObjectAsAny() const;

	private:

		ReflectableHandle object;
		TypeInfo* objectType;
	};
}