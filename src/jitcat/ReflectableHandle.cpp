/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ReflectableHandle.h"
#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/Reflectable.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeInfo.h"

#include <cassert>
#include <iostream>

using namespace jitcat::Reflection;


ReflectableHandle::ReflectableHandle():
	reflectable(nullptr),
	previousHandle(nullptr),
	nextHandle(nullptr),
	reflectableType(nullptr)
{
}


ReflectableHandle::ReflectableHandle(unsigned char* reflectable, TypeInfo* reflectableType):
	reflectable(reflectable),
	reflectableType(reflectableType),
	previousHandle(nullptr),
	nextHandle(nullptr)
{
	assert(reflectable == nullptr || reflectableType != nullptr);
	registerObserver();
}


ReflectableHandle::ReflectableHandle(const ReflectableHandle& other):
	reflectable(other.reflectable),
	previousHandle(nullptr),
	nextHandle(nullptr),
	reflectableType(other.reflectableType)
{
	registerObserver();
}


ReflectableHandle::~ReflectableHandle()
{
	deregisterObserver();
}


unsigned char* ReflectableHandle::get() const
{
	return reflectable;
}


bool ReflectableHandle::getIsValid() const
{
	return reflectable != nullptr;
}


ReflectableHandle& ReflectableHandle::operator=(const ReflectableHandle& other)
{
	if (other.reflectable != reflectable)
	{
		deregisterObserver();
		reflectable = other.reflectable;
		reflectableType = other.reflectableType;
		registerObserver();
	}
	return *this;
}


void ReflectableHandle::setReflectable(unsigned char* other, TypeInfo* reflectableType_)
{
	assert(other == nullptr || reflectableType_ != nullptr);
	if (reflectable != other)
	{
		deregisterObserver();
		reflectable = other;
		reflectableType = reflectableType_;
		registerObserver();
	}
}


ReflectableHandle& jitcat::Reflection::ReflectableHandle::operator=(std::nullptr_t other)
{
	assert(other == nullptr);
	setReflectable(nullptr, nullptr);
	return *this;
}


void ReflectableHandle::replaceCustomObjects(unsigned char* oldObject, CustomTypeInfo* oldType, unsigned char* newObject, CustomTypeInfo* newType)
{
	//All the handles that now point to oldObject should be updated to point to newObject
	//oldObject should have no handles pointing to it afterwards.
	//if newObject already has handles pointing to it, the handles that previously pointed to oldObject should be appended to newObject's
	//linked list of handles.
	if (oldType->getTypeSize() > 0)
	{
		ReflectableHandle* oldFirstHandle = getFirstHandle(oldObject, oldType->getHandleTrackingMethod());
		setFirstHandle(oldObject, nullptr, oldType->getHandleTrackingMethod());
		ReflectableHandle* newFirstHandle = getFirstHandle(newObject, newType->getHandleTrackingMethod());
		assert(oldFirstHandle == nullptr || oldFirstHandle != newFirstHandle);
		ReflectableHandle* currentHandle = oldFirstHandle;
		ReflectableHandle* lastHandle = oldFirstHandle;
		while (currentHandle != nullptr)
		{
			currentHandle->setReflectableReplacement(newObject, newType);
			currentHandle = currentHandle->getNextHandle();
			if (currentHandle != nullptr)
			{
				lastHandle = currentHandle;
			}
		}
		if (newFirstHandle != nullptr && lastHandle != nullptr)
		{
			newFirstHandle->setPreviousHandle(lastHandle);
			lastHandle->setNextHandle(newFirstHandle);
		}
		setFirstHandle(newObject, oldFirstHandle, newType->getHandleTrackingMethod());
	}
	else
	{
		setFirstHandle(newObject, nullptr, newType->getHandleTrackingMethod());
	}
}


void ReflectableHandle::nullifyObjectHandles(unsigned char* object, CustomTypeInfo* objectType)
{
	ReflectableHandle* currentHandle = getFirstHandle(object, objectType->getHandleTrackingMethod());
	while (currentHandle != nullptr)
	{
		currentHandle->setReflectableReplacement(nullptr, nullptr);
		ReflectableHandle* nextHandle = currentHandle->getNextHandle();
		currentHandle->setNextHandle(nullptr);
		currentHandle->setPreviousHandle(nullptr);
		currentHandle = nextHandle;
	}
	setFirstHandle(object, nullptr, objectType->getHandleTrackingMethod());
}


TypeInfo* jitcat::Reflection::ReflectableHandle::getObjectType() const
{
	return reflectableType;
}


ReflectableHandle* ReflectableHandle::getNextHandle() const
{
	return nextHandle;
}


void ReflectableHandle::setNextHandle(ReflectableHandle* next)
{
	nextHandle = next;
	assert(nextHandle == nullptr || nextHandle != previousHandle);
}


ReflectableHandle* ReflectableHandle::getPreviousHandle() const
{
	return previousHandle;
}


void ReflectableHandle::setPreviousHandle(ReflectableHandle* previous)
{
	previousHandle = previous;
	assert(previousHandle == nullptr || nextHandle != previousHandle);
}


void ReflectableHandle::setReflectableReplacement(unsigned char* reflectable_, TypeInfo* reflectableType_)
{
	if (reflectable != reflectable_)
	{
		reflectable = reflectable_;
		reflectableType = reflectableType_;
	}
}


bool ReflectableHandle::validateHandles() const
{
	if (reflectable != nullptr)
	{
		if (!reflectableType->isCustomType())
		{
			Reflectable* actualReflectable = reflectableType->getTypeCaster()->castToReflectable(reflectable);
			if (actualReflectable != nullptr)
			{
				return actualReflectable->validateHandles(reflectableType);
			}
		}
		else
		{
			ReflectableHandle* currentHandle = getFirstHandle(reflectable, static_cast<CustomTypeInfo*>(reflectableType)->getHandleTrackingMethod());
			while (currentHandle != nullptr)
			{
				if (currentHandle->getNextHandle() == currentHandle->getPreviousHandle() 
					&& currentHandle->getNextHandle() != nullptr)
				{
					return false;
				}
				if (currentHandle->get() != reflectable)
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
		}
	}
	return true;
}


void ReflectableHandle::registerObserver()
{
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		assert(validateHandles());
	}
	if (reflectable != nullptr)
	{
		if (!reflectableType->isCustomType())
		{
			Reflectable* actualReflectable = reflectableType->getTypeCaster()->castToReflectable(reflectable);
			if (actualReflectable != nullptr)
			{
				actualReflectable->addObserver(this);
			}
		}
		else
		{
			//Custom typed objects do not inherit from Reflectable and are tracked differently
		 	switch (static_cast<CustomTypeInfo*>(reflectableType)->getHandleTrackingMethod())
			{
				//A pointer to the first ReflectableHandle is stored as the first member of the CustomObject
				case HandleTrackingMethod::InternalHandlePointer:
				{
					ReflectableHandle** firstHandlePtr = reinterpret_cast<ReflectableHandle**>(reflectable); 
					replaceFirstHandle(firstHandlePtr, this);
				} break;
				//A pointer to the first ReflectableHandle is stored in the customObjectObservers
				case HandleTrackingMethod::ExternallyTracked:
				{
					auto iter = customObjectObservers->find(reflectable);
					if (iter == customObjectObservers->end())
					{
						customObjectObservers->insert(std::make_pair(reflectable, this));
					}
					else
					{
						replaceFirstHandle(&iter->second, this);
					}
				} break;
				case HandleTrackingMethod::None: 
				default:
					break;
			}
		}
	}
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		assert(validateHandles());
	}
}


void ReflectableHandle::deregisterObserver()
{
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		assert(validateHandles());
	}
	if (reflectable != nullptr)
	{
		if (!reflectableType->isCustomType())
		{
			Reflectable* actualReflectable = reflectableType->getTypeCaster()->castToReflectable(reflectable);
			if (actualReflectable != nullptr)
			{
				actualReflectable->removeObserver(this);
			}
		}
		else
		{
			HandleTrackingMethod trackingMethod = static_cast<CustomTypeInfo*>(reflectableType)->getHandleTrackingMethod();
			if (trackingMethod != HandleTrackingMethod::None)
			{
				if (nextHandle != nullptr)
				{
					nextHandle->setPreviousHandle(previousHandle);
				}
				if (previousHandle != nullptr)
				{
					//This is not the first handle, behaviour is the same regardles of tracking method
					previousHandle->setNextHandle(nextHandle);
				}
				else
				{
		 			switch (trackingMethod)
					{
						//A pointer to the first ReflectableHandle is stored as the first member of the CustomObject
						case HandleTrackingMethod::InternalHandlePointer:
						{
							(*reinterpret_cast<ReflectableHandle**>(reflectable)) = nextHandle;
						} break;
						//A pointer to the first ReflectableHandle is stored in the customObjectObservers
						case HandleTrackingMethod::ExternallyTracked:
						{
							auto iter = customObjectObservers->find(reflectable);
							if (iter != customObjectObservers->end())
							{
								iter->second = nextHandle;
							}
							else
							{
								assert(false);
							}
						} break;
						case HandleTrackingMethod::None: 
						default:
							break;
					}
				}
				nextHandle = nullptr;
				previousHandle = nullptr;
			}

		}
	}
	assert(previousHandle == nullptr);
	assert(nextHandle == nullptr);
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		assert(validateHandles());
	}
}


void ReflectableHandle::replaceFirstHandle(ReflectableHandle** firstHandle, ReflectableHandle* replacement)
{
	if (*firstHandle != nullptr)
	{
		(*firstHandle)->setPreviousHandle(replacement);
		replacement->setNextHandle(*firstHandle);
	}
	*firstHandle = replacement;
}


ReflectableHandle* ReflectableHandle::getFirstHandle(unsigned char* object, HandleTrackingMethod trackingMethod)
{
	switch (trackingMethod)
	{
		//A pointer to the first ReflectableHandle is stored as the first member of the CustomObject
		case HandleTrackingMethod::InternalHandlePointer:
		{
			return *reinterpret_cast<ReflectableHandle**>(object); 
		} break;
		//A pointer to the first ReflectableHandle is stored in the customObjectObservers
		case HandleTrackingMethod::ExternallyTracked:
		{
			auto iter = customObjectObservers->find(object);
			if (iter != customObjectObservers->end())
			{
				return iter->second;
			}
		} break;
	}
	return nullptr;
}


void ReflectableHandle::setFirstHandle(unsigned char* object, ReflectableHandle* handle, HandleTrackingMethod trackingMethod)
{
	switch (trackingMethod)
	{
		//A pointer to the first ReflectableHandle is stored as the first member of the CustomObject
		case HandleTrackingMethod::InternalHandlePointer:
		{
			*reinterpret_cast<ReflectableHandle**>(object) = handle; 
		} break;
		//A pointer to the first ReflectableHandle is stored in the customObjectObservers
		case HandleTrackingMethod::ExternallyTracked:
		{
			auto iter = customObjectObservers->find(object);
			if (iter != customObjectObservers->end())
			{
				iter->second = handle;
			}
			else
			{
				customObjectObservers->insert(std::make_pair(object, handle));
			}
		} break;
	}
}


std::unordered_map<const unsigned char*, ReflectableHandle*>* ReflectableHandle::customObjectObservers = new std::unordered_map<const unsigned char*, ReflectableHandle*>();