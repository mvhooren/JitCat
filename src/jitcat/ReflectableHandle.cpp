/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ReflectableHandle.h"
#include "Reflectable.h"


ReflectableHandle::ReflectableHandle(Reflectable* reflectable):
	reflectable(reflectable)
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


void ReflectableHandle::notifyDeletion()
{
	reflectable = nullptr;
}
