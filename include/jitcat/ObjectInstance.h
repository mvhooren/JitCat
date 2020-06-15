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
		//Default constructs the object instance
		ObjectInstance(TypeInfo* objectType);
		//Takes ownership of the object. Use createCopy to copy construct the object.
		ObjectInstance(unsigned char* object, TypeInfo* objectType);
		//Also copy constructs the object instance
		ObjectInstance(const ObjectInstance& other);
		//Also move constructs the object instance
		ObjectInstance(ObjectInstance&& other) noexcept;
		ObjectInstance& operator=(ObjectInstance&& other) noexcept;
		~ObjectInstance();

		static ObjectInstance createCopy(unsigned char* object, TypeInfo* objectType);

		unsigned char* getObject() const;
		TypeInfo* getType() const;
		std::any getObjectAsAny() const;

	private:

		ReflectableHandle object;
		TypeInfo* objectType;
	};
}