/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Reflectable.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTraits.h"
#include <any>
#include <map>
#include <string>
#include <vector>

namespace jitcat::Reflection
{
	class ContainerManipulator
	{
	public:
		virtual ~ContainerManipulator() {}
		virtual std::size_t getContainerSize(std::any container) const = 0;
		virtual std::any getItemAt(std::any container, int index) = 0;
		virtual std::any getItemAt(std::any container, const std::string& index) = 0;
		virtual int getIndexOf(std::any container, const std::string& index) = 0;
		virtual std::any createAnyPointer(uintptr_t pointer) = 0;
		virtual std::string getKeyAtIndex(std::any container, int index) const = 0;
	};


	template<typename VectorT>
	class VectorManipulator: public ContainerManipulator
	{
	public:
		VectorManipulator() {}
		virtual ~VectorManipulator() {}
		virtual std::size_t getContainerSize(std::any container) const override final
		{
			VectorT* vector = std::any_cast<VectorT*>(container);
			if (vector != nullptr)
			{
				return vector->size();
			}
			return 0;
		}


		virtual std::any getItemAt(std::any container, int index) override final
		{
			VectorT* vector = std::any_cast<VectorT*>(container);
			if (vector != nullptr && index < (int)vector->size() && index >= 0)
			{
				return TypeTraits<typename VectorT::value_type>::getCatValue(vector->operator[](index));
			}
			else
			{
				return TypeTraits<typename VectorT::value_type>::getDefaultCatValue();
			}
		}


		virtual std::any getItemAt(std::any container, const std::string& index) override final
		{
			return TypeTraits<typename VectorT::value_type>::getDefaultCatValue();
		}


		virtual int getIndexOf(std::any container, const std::string& index) override final
		{
			return -1;
		}
		

		virtual std::any createAnyPointer(uintptr_t pointer) override final
		{
			return std::any(reinterpret_cast<VectorT*>(pointer));
		}


		virtual std::string getKeyAtIndex(std::any container, int index) const override final
		{
			return "";
		}
	};


	template<typename MapT>
	class MapManipulator : public ContainerManipulator
	{
	public:
		MapManipulator() {}
		virtual ~MapManipulator() {}

		virtual std::size_t getContainerSize(std::any container) const override final
		{
			MapT* map = std::any_cast<MapT*>(container);
			if (map != nullptr)
			{
				return map->size();
			}
			return 0;
		}


		virtual std::any getItemAt(std::any container, int index) override final
		{
			MapT* map = std::any_cast<MapT*>(container);
			if (map != nullptr && index < (int)map->size() && index >= 0)
			{
				int count = 0;
				for (auto& iter : *map)
				{
					if (count == index)
					{
						return TypeTraits<typename MapT::mapped_type>::getCatValue(iter.second);
					}
					count++;
				}
				return TypeTraits<typename MapT::mapped_type>::getDefaultCatValue();
			}
			else
			{
				return TypeTraits<typename MapT::mapped_type>::getDefaultCatValue();
			}
		}


		virtual std::any getItemAt(std::any container, const std::string& index) override final
		{
			MapT* map = std::any_cast<MapT*>(container);
			if (map != nullptr)
			{
				std::string lowerIndex = Tools::toLowerCase(index);	
				auto& iter = map->find(lowerIndex);
				if (iter != map->end())
				{
					return TypeTraits<typename MapT::mapped_type>::getCatValue(iter->second);
				}
			}
			return TypeTraits<typename MapT::mapped_type>::getDefaultCatValue();
		}


		virtual int getIndexOf(std::any container, const std::string& index) override final
		{
			MapT* map = std::any_cast<MapT*>(container);
			if (map != nullptr)
			{
				std::string lowerIndex = Tools::toLowerCase(index);
				int count = 0;
				for (auto& iter : *map)
				{
					if (iter.first == lowerIndex)
					{
						return count;
					}
					count++;
				}
			}
			return -1;
		}


		virtual std::string getKeyAtIndex(std::any container, int index) const override final
		{
			MapT* map = std::any_cast<MapT*>(container);
			if (map != nullptr && index < (int)map->size() && index >= 0)
			{
				int count = 0;
				for (auto& iter : *map)
				{
					if (count == index)
					{
						return iter.first;
					}
					count++;
				}
			}
			return "";
		}

		virtual std::any createAnyPointer(uintptr_t pointer) override final
		{
			return std::any(reinterpret_cast<MapT*>(pointer));
		}
	};

}