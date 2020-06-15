/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/TypeCaster.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ObjectInstance.h"

using namespace jitcat;
using namespace jitcat::Reflection;

CustomObjectTypeCaster::CustomObjectTypeCaster(CustomTypeInfo* customType): 
    customType(customType) 
{}


bool CustomObjectTypeCaster::isNullPtr(const std::any & value) const
{
	Reflectable* reflectable = std::any_cast<Reflectable*>(value);
	return reflectable == nullptr;
}


bool CustomObjectTypeCaster::isNullPtrPtr(const std::any& value) const
{
	Reflectable** reflectable = std::any_cast<Reflectable**>(value);
	return reflectable == nullptr;
}


void CustomObjectTypeCaster::toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const
{
	Reflectable* reflectable = std::any_cast<Reflectable*>(value);
	buffer = reinterpret_cast<const unsigned char*>(reflectable);
	bufferSize = customType->getTypeSize();
}


std::any CustomObjectTypeCaster::getValueOfPointer(std::any& value) const
{
	return std::any(ObjectInstance::createCopy(reinterpret_cast<unsigned char*>(std::any_cast<Reflectable*>(value)), customType));
}


std::any CustomObjectTypeCaster::getValueOfPointerToPointer(std::any& value) const
{
	Reflectable** ptrptr = std::any_cast<Reflectable**>(value);
	return *ptrptr;
}


std::any CustomObjectTypeCaster::getAddressOfValue(std::any& value) const
{
	return reinterpret_cast<Reflectable*>(std::any_cast<ObjectInstance>(&value)->getObject());
}


std::any CustomObjectTypeCaster::getAddressOfPointer(std::any& value) const
{
	Reflectable** addressOf = std::any_cast<Reflectable*>(&value);
	return addressOf;
}


std::any CustomObjectTypeCaster::castFromRawPointer(uintptr_t pointer) const
{
	return reinterpret_cast<Reflectable*>(pointer);
}


uintptr_t CustomObjectTypeCaster::castToRawPointer(const std::any& pointer) const
{
	return reinterpret_cast<uintptr_t>(std::any_cast<Reflectable*>(pointer));
}


std::any CustomObjectTypeCaster::castFromRawPointerPointer(uintptr_t pointer) const
{
	return reinterpret_cast<Reflectable**>(pointer);
}


uintptr_t CustomObjectTypeCaster::castToRawPointerPointer(const std::any& pointer) const
{
	return reinterpret_cast<uintptr_t>(std::any_cast<Reflectable**>(pointer));
}


std::any CustomObjectTypeCaster::getNull() const
{
	return static_cast<Reflectable*>(nullptr);
}


bool NullptrTypeCaster::isNullPtr(const std::any& value) const
{
	return true;
}


bool NullptrTypeCaster::isNullPtrPtr(const std::any& value) const
{
	return true;
}


std::any NullptrTypeCaster::getValueOfPointer(std::any& value) const
{
	return nullptr;
}


std::any NullptrTypeCaster::getValueOfPointerToPointer(std::any& value) const
{
	return nullptr;
}


std::any NullptrTypeCaster::getAddressOfValue(std::any& value) const
{
	return nullptr;
}


std::any NullptrTypeCaster::getAddressOfPointer(std::any& value) const
{
	return nullptr;
}


std::any NullptrTypeCaster::castFromRawPointer(uintptr_t pointer) const
{
	return getNull();
}


uintptr_t NullptrTypeCaster::castToRawPointer(const std::any& pointer) const
{
	return reinterpret_cast<uintptr_t>(nullptr);
}


std::any NullptrTypeCaster::castFromRawPointerPointer(uintptr_t pointer) const
{
	return nullptr;
}


uintptr_t NullptrTypeCaster::castToRawPointerPointer(const std::any& pointer) const
{
	return reinterpret_cast<uintptr_t>(nullptr);
}


void NullptrTypeCaster::toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const
{
	buffer = nullptr;
	bufferSize = 0;
}


std::any NullptrTypeCaster::getNull() const
{
	return nullptr;
}
