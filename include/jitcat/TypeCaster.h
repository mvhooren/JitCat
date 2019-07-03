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
	virtual std::any cast(const std::any& pointer) const = 0;
	virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const = 0;
	virtual std::any cast(uintptr_t pointer) const = 0;
	virtual std::any getNull() const = 0;
};


template<typename ObjectT>
class ObjectTypeCaster: public TypeCaster
{
public:
	ObjectTypeCaster() {};
	virtual ~ObjectTypeCaster() {};


	virtual std::any cast(const std::any& pointer ) const override final
	{
		return std::any(static_cast<Reflectable*>(std::any_cast<ObjectT*>(pointer)));
	}

	virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const override final
	{
		Reflectable* reflectable = std::any_cast<Reflectable*>(value);
		buffer = reinterpret_cast<const unsigned char*>(reflectable);
		bufferSize = sizeof(ObjectT);
	}


	virtual std::any cast(uintptr_t pointer) const override final
	{
		return std::any(static_cast<Reflectable*>(reinterpret_cast<ObjectT*>(pointer)));
	}


	virtual std::any getNull() const override final 
	{
		return static_cast<Reflectable*>((ObjectT*)nullptr);
	}

};


class CustomObjectTypeCaster: public TypeCaster
{
public:
	CustomObjectTypeCaster(CustomTypeInfo* customType): customType(customType) {};
	virtual ~CustomObjectTypeCaster() {};


	virtual std::any cast(const std::any& pointer) const override final
	{
		return pointer;
	}

	virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const override final
	{
		Reflectable* reflectable = std::any_cast<Reflectable*>(value);
		buffer = reinterpret_cast<const unsigned char*>(reflectable);
		bufferSize = customType->getTypeSize();
	}

	virtual std::any cast(uintptr_t pointer) const override final
	{
		return std::any(reinterpret_cast<Reflectable*>(pointer));
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


	virtual std::any cast(const std::any& pointer) const override final
	{
		return getNull();
	}

	
	virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const override final
	{
		buffer = nullptr;
		bufferSize = 0;
	}


	virtual std::any cast(uintptr_t pointer) const override final
	{
		return getNull();
	}


	virtual std::any getNull() const override final
	{
		return static_cast<Reflectable*>(nullptr);
	}

};


} //End namespace jitcat::Reflection