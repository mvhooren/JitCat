/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <any>
#include <map>
#include <string>
#include <vector>


class TypeCaster
{
public:
	TypeCaster() {};
	virtual ~TypeCaster() {};
	virtual std::any cast(uintptr_t pointer) const = 0;
	virtual std::any castToVectorOf(uintptr_t pointer) const = 0;
	virtual std::any castToStringIndexedMapOf(uintptr_t pointer) const = 0;
};


template<typename ObjectT>
class ObjectTypeCaster: public TypeCaster
{
public:
	ObjectTypeCaster() {};
	virtual ~ObjectTypeCaster() {};
	virtual std::any cast(uintptr_t pointer) const override final
	{
		return std::any(reinterpret_cast<ObjectT*>(pointer));
	}
	virtual std::any castToVectorOf(uintptr_t pointer) const override final
	{
		return std::any(reinterpret_cast<std::vector<ObjectT*>*>(pointer));
	}
	virtual std::any castToStringIndexedMapOf(uintptr_t pointer) const override final
	{
		return std::any(reinterpret_cast<std::map<std::string, ObjectT*>*>(pointer));
	}
};