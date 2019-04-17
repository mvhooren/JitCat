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
	virtual std::any getNullVector() const = 0;
	virtual std::any getNullStringIndexedMap() const = 0;

	virtual std::any castToVectorOf(uintptr_t pointerToVector) const = 0;
	virtual int getSizeOfVectorOf(std::any pointerToVector) const = 0;
	virtual std::any getItemOfVectorOf(std::any pointerToVector, int index) const = 0;

	virtual std::any castToStringIndexedMapOf(uintptr_t pointerToMap) const = 0;
	virtual int sizeOfStringIndexedMapOf(std::any pointerToMap) const = 0;
	virtual std::any getItemOfStringIndexedMapOf(std::any pointerToMap, int index) const = 0;
	virtual std::any getItemOfStringIndexedMapOf(std::any pointerToMap, const std::string& index) const = 0;
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


	virtual std::any getNullVector() const
	{
		return (std::vector<ObjectT*>*)(nullptr);
	}


	virtual std::any getNullStringIndexedMap() const 
	{
		return (std::map<std::string, ObjectT*>*)(nullptr);
	}


	virtual std::any castToVectorOf(uintptr_t pointer) const override final
	{
		return std::any(reinterpret_cast<std::vector<ObjectT*>*>(pointer));
	}


	virtual int getSizeOfVectorOf(std::any pointerToVector) const override final
	{
		std::vector<ObjectT*>* vector = std::any_cast<std::vector<ObjectT*>*>(pointerToVector);
		if (vector != 0)
		{
			return (int)vector->size();
		}
		else
		{
			return 0;
		}
	}


	virtual std::any getItemOfVectorOf(std::any pointerToVector, int index) const override final
	{
		std::vector<ObjectT*>* vector = std::any_cast<std::vector<ObjectT*>*>(pointerToVector);
		if (vector != nullptr && index >= 0 && index < (int)vector->size())
		{
			return std::any(static_cast<Reflectable*>(vector->operator[](index)));
		}
		return getNull();
	}


	virtual std::any castToStringIndexedMapOf(uintptr_t pointer) const override final
	{
		return std::any(reinterpret_cast<std::map<std::string, ObjectT*>*>(pointer));
	}


	virtual int sizeOfStringIndexedMapOf(std::any pointerToMap) const override final
	{
		std::map<std::string, ObjectT*>* map = std::any_cast<std::map<std::string, ObjectT*>*>(pointerToMap);
		if (map != nullptr)
		{
			return (int)map->size();
		}
		else
		{
			return 0;
		}
	}


	virtual std::any getItemOfStringIndexedMapOf(std::any pointerToMap, int index) const override final
	{
		std::map<std::string, ObjectT*>* map = std::any_cast<std::map<std::string, ObjectT*>*>(pointerToMap);
		if (map != nullptr)
		{
			auto mapEnd = map->end();
			int count = 0;
			for (auto iter = map->begin(); iter != mapEnd; ++iter)
			{
				if (index == count)
				{
					return static_cast<Reflectable*>(iter->second);
				}
				count++;
			}
		}
		return getNull();
	}


	virtual std::any getItemOfStringIndexedMapOf(std::any pointerToMap, const std::string& index) const override final
	{
		auto map = std::any_cast<std::map<std::string, ObjectT*>*>(pointerToMap);
		if (map != nullptr)
		{
			std::string lowercaseIndex = Tools::toLowerCase(index);
			auto iter = map->find(lowercaseIndex);
			if (iter != map->end())
			{
				return static_cast<Reflectable*>(iter->second);
			}
		}
		return getNull();
	}

};


} //End namespace jitcat::Reflection