/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ObjectInstance.h"
#include "jitcat/Reflectable.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


ObjectInstance::ObjectInstance():
	object(nullptr),
	objectType(nullptr)
{
}


ObjectInstance::ObjectInstance(TypeInfo* objectType):
	object(reinterpret_cast<Reflectable*>(objectType->construct())),
	objectType(objectType)
{
	assert(objectType->getAllowConstruction());
}


ObjectInstance::ObjectInstance(unsigned char* object, TypeInfo* objectType):
	object(reinterpret_cast<Reflectable*>(object)),
	objectType(objectType)
{
	assert(objectType->getAllowConstruction());
}


ObjectInstance::ObjectInstance(const ObjectInstance& other):
	objectType(other.objectType)
{
	assert(objectType->getAllowCopyConstruction());
	unsigned char* objectBuffer = new unsigned char[objectType->getTypeSize()];
	objectType->copyConstruct(objectBuffer, objectType->getTypeSize(), other.getObject(), objectType->getTypeSize());
	object = reinterpret_cast<Reflectable*>(objectBuffer);
}


ObjectInstance::ObjectInstance(ObjectInstance&& other) noexcept :
	objectType(other.objectType),
	object(nullptr)
{
	if (&other != this)
	{
		object = other.object;
		objectType = other.objectType;
		other.object = nullptr;
		other.objectType = nullptr;
	}
}


ObjectInstance::~ObjectInstance()
{
	assert(objectType == nullptr || objectType->getAllowConstruction());
	if (objectType != nullptr && object.getIsValid())
	{
		objectType->destruct(reinterpret_cast<unsigned char*>(object.get()));
		object = nullptr;
	}
}


ObjectInstance& ObjectInstance::operator=(ObjectInstance&& other) noexcept
{
	if (objectType != nullptr && object.getIsValid())
	{
		assert(objectType->getAllowConstruction());
		objectType->destruct(reinterpret_cast<unsigned char*>(object.get()));
		object = nullptr;
	}
	if (&other != this)
	{
		object = other.object;
		objectType = other.objectType;
		other.object = nullptr;
		other.objectType = nullptr;
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
	return objectType;
}


std::any jitcat::Reflection::ObjectInstance::getObjectAsAny() const
{
	return objectType->getTypeCaster()->castFromRawPointer(reinterpret_cast<uintptr_t>(object.get()));
}
