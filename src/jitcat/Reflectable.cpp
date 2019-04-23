/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Reflectable.h"
#include "jitcat/ReflectableHandle.h"

using namespace jitcat::Reflection;


jitcat::Reflection::Reflectable::Reflectable(const Reflectable& other)
{
	//Observers are not copied.
	//Use move constructor if observers should be copied and updated.
}


Reflectable::Reflectable()
{
}


Reflectable::Reflectable(Reflectable&& other) noexcept
{
	while (!other.observers.empty())
	{
		(*other.observers.begin())->operator=(this);
	}
}


Reflectable& jitcat::Reflection::Reflectable::operator=(const Reflectable& other)
{
	//Observers are not copied.
	//Use move constructor if observers should be copied and updated.
	return *this;
}


Reflectable::~Reflectable()
{
	for (unsigned int i = 0; i < observers.size(); i++)
	{
		observers[i]->notifyDeletion();
	}
}


void Reflectable::addObserver(ReflectableHandle* observer)
{
	observers.push_back(observer);
}


void Reflectable::removeObserver(ReflectableHandle* observer)
{
	for (unsigned int i = 0; i < observers.size(); i++)
	{
		if (observers[i] == observer)
		{
			observers.erase(observers.begin() + (int)i);
			break;
		}
	}
}