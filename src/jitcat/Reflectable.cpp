/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Reflectable.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeInfo.h"

#include <algorithm>
#include <cassert>
#include <iostream>
using namespace jitcat::Reflection;


Reflectable::Reflectable(const Reflectable& other):
	firstHandle(nullptr)
{
	//Observers are not copied.
	//Use move constructor if observers should be copied and updated.
}


Reflectable::Reflectable():
	firstHandle(nullptr)
{
}


Reflectable::Reflectable(Reflectable&& other) noexcept:
	firstHandle(other.firstHandle)
{
	if constexpr (Configuration::enableHandleVerificationAsserts)
		if (other.firstHandle != nullptr)
				assert(other.validateHandles(other.firstHandle->getObjectType()));

	other.firstHandle = nullptr;
	ReflectableHandle* currentHandle = firstHandle;
	while (currentHandle != nullptr)
	{
		unsigned char* object = currentHandle->getObjectType()->getTypeCaster()->castToObject(this);
		currentHandle->setReflectableReplacement(object, currentHandle->getObjectType());
		currentHandle = currentHandle->getNextHandle();
	}
	if constexpr (Configuration::enableHandleVerificationAsserts)
		if (firstHandle != nullptr)
			assert(validateHandles(firstHandle->getObjectType()));
}


Reflectable& jitcat::Reflection::Reflectable::operator=(const Reflectable& other)
{
	//Observers are not copied.
	//Use move constructor if observers should be copied and updated.
	return *this;
}


Reflectable::~Reflectable()
{
	if constexpr (Configuration::enableHandleVerificationAsserts)
		if (firstHandle != nullptr)
			assert(validateHandles(firstHandle->getObjectType()));

	ReflectableHandle* currentHandle = firstHandle;
	while (currentHandle != nullptr)
	{
		currentHandle->setReflectableReplacement(nullptr, nullptr);
		ReflectableHandle* previousHandle = currentHandle;
		currentHandle = currentHandle->getNextHandle();
		previousHandle->setNextHandle(nullptr);
		previousHandle->setPreviousHandle(nullptr);
	}
}


void Reflectable::addObserver(ReflectableHandle* observer)
{
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		if (firstHandle != nullptr)
		{
			assert(validateHandles(firstHandle->getObjectType()));
			assert(!hasHandle(observer));
		}
	}
	if (firstHandle == nullptr)
	{
		firstHandle = observer;
	}
	else
	{
		firstHandle->setPreviousHandle(observer);
		observer->setNextHandle(firstHandle);
		firstHandle = observer;
	}
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		assert(hasHandle(observer));
		assert(validateHandles(firstHandle->getObjectType()));
	}
}


void Reflectable::removeObserver(ReflectableHandle* observer)
{
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		if (firstHandle != nullptr)
		{
			assert(validateHandles(firstHandle->getObjectType()));
		}
		assert(hasHandle(observer));
	}
	ReflectableHandle* nextHandle = observer->getNextHandle();
	if (nextHandle != nullptr)
	{
		nextHandle->setPreviousHandle(observer->getPreviousHandle());
	}
	if (observer->getPreviousHandle() != nullptr)
	{
		observer->getPreviousHandle()->setNextHandle(nextHandle);
	}
	else
	{
		firstHandle = nextHandle;
	}
	observer->setPreviousHandle(nullptr);
	observer->setNextHandle(nullptr);
	assert(!hasHandle(observer));
	if (firstHandle != nullptr)
	{	
		assert(validateHandles(firstHandle->getObjectType()));
	}
}


std::size_t Reflectable::getNumReflectableHandles() const
{
	ReflectableHandle* currentHandle = firstHandle;
	std::size_t count = 0;
	while (currentHandle != nullptr)
	{
		count++;
		currentHandle = currentHandle->getNextHandle();
	}
	return count;
}


bool Reflectable::validateHandles(TypeInfo* reflectableType)
{
	if (this == nullptr)
	{
		return true;
	}
	const ReflectableHandle* currentHandle = firstHandle;
	while (currentHandle != nullptr)
	{
		if (currentHandle->getNextHandle() == currentHandle->getPreviousHandle() 
			&& currentHandle->getNextHandle() != nullptr)
		{
			return false;
		}
		if (currentHandle->get() != reflectableType->getTypeCaster()->castToObject(this))
		{
			return false;
		}
		if (currentHandle->getNextHandle() != nullptr 
			&& currentHandle->getNextHandle()->getPreviousHandle() != currentHandle)
		{
			return false;
		}
		currentHandle = currentHandle->getNextHandle();
	}
	return true;
}


bool Reflectable::hasHandle(ReflectableHandle* handle) const
{
	const ReflectableHandle* currentHandle = firstHandle;
	while (currentHandle != nullptr)
	{
		if (currentHandle == handle)
		{
			return true;
		}
		currentHandle = currentHandle->getNextHandle();
	}
	return false;
}