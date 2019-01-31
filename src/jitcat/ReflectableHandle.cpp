/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ReflectableHandle.h"
#include "jitcat/Reflectable.h"

using namespace jitcat::Reflection;

ReflectableHandle::ReflectableHandle():
	reflectable(nullptr)
{
}


ReflectableHandle::ReflectableHandle(Reflectable* reflectable):
	reflectable(reflectable)
{
	if (reflectable != nullptr)
	{
		reflectable->addObserver(this);
	}
}


ReflectableHandle::ReflectableHandle(const ReflectableHandle& other):
	reflectable(other.reflectable)
{
	if (reflectable != nullptr)
	{
		reflectable->addObserver(this);
	}
}


ReflectableHandle::~ReflectableHandle()
{
	if (reflectable != nullptr)
	{
		reflectable->removeObserver(this);
	}
}


Reflectable* ReflectableHandle::get() const
{
	return reflectable;
}


bool ReflectableHandle::getIsValid() const
{
	return reflectable != nullptr;
}


ReflectableHandle& ReflectableHandle::operator=(const ReflectableHandle& other)
{
	if (reflectable != nullptr)
	{
		reflectable->removeObserver(this);
	}
	reflectable = other.reflectable;
	if (reflectable != nullptr)
	{
		reflectable->addObserver(this);
	}
	return *this;
}


ReflectableHandle& ReflectableHandle::operator=(Reflectable* other)
{
	if (reflectable != nullptr)
	{
		reflectable->removeObserver(this);
	}
	reflectable = other;
	if (reflectable != nullptr)
	{
		reflectable->addObserver(this);
	}
	return *this;
}


void ReflectableHandle::notifyDeletion()
{
	reflectable = nullptr;
}


Reflectable* ReflectableHandle::staticGet(const ReflectableHandle& handle)
{
	return handle.get();
}


void ReflectableHandle::staticAssign(ReflectableHandle& handle, Reflectable* reflectable)
{
	handle = reflectable;
}
