/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ReflectableInstance.h"
#include "jitcat/Reflectable.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


jitcat::Reflection::ReflectableInstance::ReflectableInstance():
	reflectable(nullptr),
	reflectableType(nullptr)
{
}


jitcat::Reflection::ReflectableInstance::ReflectableInstance(Reflectable* reflectable, TypeInfo* reflectableType):
	reflectable(reflectable),
	reflectableType(reflectableType)
{
}


jitcat::Reflection::ReflectableInstance::~ReflectableInstance()
{
	if (reflectableType != nullptr && reflectable.getIsValid())
	{
		reflectableType->destruct(reflectable.get());
		reflectable = nullptr;
	}
}


ReflectableInstance& jitcat::Reflection::ReflectableInstance::operator=(ReflectableInstance&& other) noexcept
{
	if (&other != this)
	{
		reflectable = other.reflectable;
		reflectableType = other.reflectableType;
		other.reflectable = nullptr;
		other.reflectableType = nullptr;
	}
	return *this;
}


Reflectable* jitcat::Reflection::ReflectableInstance::getReflectable() const
{
	return reflectable.get();
}


TypeInfo* jitcat::Reflection::ReflectableInstance::getType() const
{
	return reflectableType;
}
