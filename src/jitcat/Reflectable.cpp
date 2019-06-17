/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Reflectable.h"
#include "jitcat/ReflectableHandle.h"

#include <algorithm>

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
	while (true)
	{
		if (auto& iter = observers.find(&other); iter != observers.end())
		{
			iter->second->operator=(this);
		}
		else
		{
			break;
		}
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
	while (true)
	{
		if (auto& iter = observers.find(this); iter != observers.end())
		{
			iter->second->notifyDeletion();
			observers.erase(iter);
		}
		else
		{
			break;
		}
	}
}


void Reflectable::addObserver(ReflectableHandle* observer)
{
	observers.insert(std::pair<Reflectable*,ReflectableHandle*>(this, observer));
}


void Reflectable::removeObserver(ReflectableHandle* observer)
{
	auto& range = observers.equal_range(this);
	for (auto& iter = range.first; iter != range.second; ++iter)
	{
		if (iter->second == observer)
		{
			observers.erase(iter);
			break;
		}
	}
}


void jitcat::Reflection::Reflectable::destruct(Reflectable* reflectable)
{
	delete reflectable;
}


std::unordered_multimap<Reflectable*, ReflectableHandle*> Reflectable::observers = std::unordered_multimap<Reflectable*, ReflectableHandle*>();