/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once
#include "jitcat/Reflectable.h"
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


	virtual std::any cast(uintptr_t pointer) const override final
	{
		return std::any(static_cast<Reflectable*>(reinterpret_cast<ObjectT*>(pointer)));
	}


	virtual std::any getNull() const override final 
	{
		return static_cast<Reflectable*>((ObjectT*)nullptr);
	}

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