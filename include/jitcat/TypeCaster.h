/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once
#include "jitcat/Reflectable.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/Tools.h"

#include <any>
#include <cassert>
#include <map>
#include <string>
#include <vector>


namespace jitcat::Reflection
{

class TypeCaster
{
public:
	TypeCaster() {};
	virtual ~TypeCaster() {};
	virtual std::any getAddressOfValue(std::any& value) const = 0;
	virtual std::any getAddressOfPointer(std::any& value) const = 0;
	virtual std::any castFromRawPointer(uintptr_t pointer) const = 0;
	virtual uintptr_t castToRawPointer(const std::any& pointer) const = 0;
	virtual std::any castFromRawPointerPointer(uintptr_t pointer) const = 0;
	virtual uintptr_t castToRawPointerPointer(const std::any& pointer) const = 0;

	virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const = 0;
	virtual std::any getNull() const = 0;
};


template<typename ObjectT>
class ObjectTypeCaster: public TypeCaster
{
public:
	ObjectTypeCaster() {};
	virtual ~ObjectTypeCaster() {};

	virtual std::any getAddressOfValue(std::any& value) const override final
	{
		ObjectT* addressOf = std::any_cast<ObjectT>(&value);
		return addressOf;
	}


	virtual std::any getAddressOfPointer(std::any& value) const override final
	{
		ObjectT** addressOf = std::any_cast<ObjectT*>(&value);
		return addressOf;
	}


	virtual std::any castFromRawPointer(uintptr_t pointer) const override final
	{
		return reinterpret_cast<ObjectT*>(pointer);
	}


	virtual uintptr_t castToRawPointer(const std::any& pointer) const override final
	{
		return reinterpret_cast<uintptr_t>(std::any_cast<ObjectT*>(pointer));
	}


	virtual std::any castFromRawPointerPointer(uintptr_t pointer) const override final
	{
		return reinterpret_cast<ObjectT**>(pointer);
	}


	virtual uintptr_t castToRawPointerPointer(const std::any& pointer) const override final
	{
		return reinterpret_cast<uintptr_t>(std::any_cast<ObjectT**>(pointer));
	}


	virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const override final
	{
		ObjectT* object = std::any_cast<ObjectT*>(value);
		buffer = reinterpret_cast<const unsigned char*>(object);
		bufferSize = sizeof(ObjectT);
	}


	virtual std::any getNull() const override final 
	{
		return (ObjectT*)nullptr;
	}

};


class CustomObjectTypeCaster: public TypeCaster
{
public:
	CustomObjectTypeCaster(CustomTypeInfo* customType): customType(customType) {};
	virtual ~CustomObjectTypeCaster() {};


	virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const override final
	{
		Reflectable* reflectable = std::any_cast<Reflectable*>(value);
		buffer = reinterpret_cast<const unsigned char*>(reflectable);
		bufferSize = customType->getTypeSize();
	}


	virtual std::any getAddressOfValue(std::any& value) const override final
	{
		//Custom objects can not be passed by value at this time
		assert(false);
		return getNull();
	}


	virtual std::any getAddressOfPointer(std::any& value) const override final
	{
		Reflectable** addressOf = std::any_cast<Reflectable*>(&value);
		return addressOf;
	}


	virtual std::any castFromRawPointer(uintptr_t pointer) const override final
	{
		return reinterpret_cast<Reflectable*>(pointer);
	}

	
	virtual uintptr_t castToRawPointer(const std::any& pointer) const override final
	{
		return reinterpret_cast<uintptr_t>(std::any_cast<Reflectable*>(pointer));
	}


	virtual std::any castFromRawPointerPointer(uintptr_t pointer) const override final
	{
		return reinterpret_cast<Reflectable**>(pointer);
	}


	virtual uintptr_t castToRawPointerPointer(const std::any& pointer) const override final
	{
		return reinterpret_cast<uintptr_t>(std::any_cast<Reflectable**>(pointer));
	}


	virtual std::any getNull() const override final
	{
		return static_cast<Reflectable*>(nullptr);
	}

private:
	CustomTypeInfo* customType;
};


class NullptrTypeCaster: public TypeCaster
{
public:
	NullptrTypeCaster() {};
	virtual ~NullptrTypeCaster() {};
	
	
	virtual std::any getAddressOfValue(std::any& value) const override final
	{
		return nullptr;
	}


	virtual std::any getAddressOfPointer(std::any& value) const override final
	{
		return nullptr;
	}


	virtual std::any castFromRawPointer(uintptr_t pointer) const override final
	{
		return getNull();
	}

	
	virtual uintptr_t castToRawPointer(const std::any& pointer) const override final
	{
		return reinterpret_cast<uintptr_t>(nullptr);
	}


	virtual std::any castFromRawPointerPointer(uintptr_t pointer) const override final
	{
		return nullptr;
	}


	virtual uintptr_t castToRawPointerPointer(const std::any& pointer) const override final
	{
		return reinterpret_cast<uintptr_t>(nullptr);
	}


	virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const override final
	{
		buffer = nullptr;
		bufferSize = 0;
	}


	virtual std::any getNull() const override final
	{
		return nullptr;
	}

};


} //End namespace jitcat::Reflection