/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Configuration.h"
#include "jitcat/ObjectInstance.h"
#include "jitcat/Reflectable.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


ObjectInstance::ObjectInstance()
{
}


ObjectInstance::ObjectInstance(TypeInfo* objectType):
	object(objectType->construct(), objectType)
{
	assert(objectType->getAllowConstruction());
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		assert(object.validateHandles());
	}
}


ObjectInstance::ObjectInstance(unsigned char* object, TypeInfo* objectType):
	object(object, objectType)
{
	assert(objectType->getAllowConstruction());
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		assert(this->object.validateHandles());
	}
}


ObjectInstance::ObjectInstance(const ObjectInstance& other)
{
	assert(other.getType()->getAllowCopyConstruction());
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		assert(other.validateHandle());
	}
	unsigned char* objectBuffer = new unsigned char[other.getType()->getTypeSize()];
	other.getType()->copyConstruct(objectBuffer, other.getType()->getTypeSize(), other.getObject(), other.getType()->getTypeSize());
	object.setReflectable(objectBuffer, other.getType());
}


ObjectInstance::ObjectInstance(ObjectInstance&& other) noexcept
{
	if (&other != this)
	{
		object = other.object;
		other.object = nullptr;
	}
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		assert(object.validateHandles());
	}
}


ObjectInstance::~ObjectInstance()
{
	assert(object.getObjectType() == nullptr || object.getObjectType()->getAllowConstruction());
	if (object.getObjectType() != nullptr && object.getIsValid())
	{
		object.getObjectType()->destruct(reinterpret_cast<unsigned char*>(object.get()));
		object = nullptr;
	}
}


ObjectInstance& ObjectInstance::operator=(ObjectInstance&& other) noexcept
{
	if (object.getObjectType() != nullptr && object.getIsValid())
	{
		assert(object.getObjectType()->getAllowConstruction());
		object.getObjectType()->destruct(reinterpret_cast<unsigned char*>(object.get()));
		object = nullptr;
	}
	if (&other != this)
	{
		object = other.object;
		other.object = nullptr;
	}
	return *this;
}


ObjectInstance ObjectInstance::createCopy(unsigned char* object, TypeInfo* objectType)
{
	assert(objectType->getAllowCopyConstruction());
	unsigned char* objectBuffer = new unsigned char[objectType->getTypeSize()];
	objectType->copyConstruct(objectBuffer, objectType->getTypeSize(), object, objectType->getTypeSize());
	return ObjectInstance(objectBuffer, objectType);
}


unsigned char* ObjectInstance::getObject() const
{
	return reinterpret_cast<unsigned char*>(object.get());
}


TypeInfo* ObjectInstance::getType() const
{
	return object.getObjectType();
}


std::any ObjectInstance::getObjectAsAny() const
{
	return object.getObjectType()->getTypeCaster()->castFromRawPointer(reinterpret_cast<uintptr_t>(object.get()));
}


bool ObjectInstance::validateHandle() const
{
	if constexpr (Configuration::enableHandleVerificationAsserts)
	{
		return object.validateHandles();
	}
	else
	{
		return true;
	}
}
