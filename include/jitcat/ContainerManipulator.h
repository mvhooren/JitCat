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
		virtual std::any getItemAt(std::any container, std::any key) = 0;
		virtual int getIndexOf(std::any container, std::any key) = 0;
		virtual std::any createAnyPointer(uintptr_t pointer) = 0;
		virtual std::any getKeyAtIndex(std::any container, int index) const = 0;
		virtual CatGenericType getKeyType() const = 0;
		virtual CatGenericType getValueType() const = 0;
	};


	//Used for deserialized type information, not backed by actual reflected type.
	class DummyManipulator: public ContainerManipulator
	{
	public:
		DummyManipulator(const CatGenericType& keyType, const CatGenericType& valueType): keyType(keyType), valueType(valueType) {}
		virtual std::size_t getContainerSize(std::any container) const {return 0;}
		virtual std::any getItemAt(std::any container, int index) {	return std::any();}
		virtual std::any getItemAt(std::any container, std::any key) { return std::any(); }
		virtual int getIndexOf(std::any container, std::any key) { return -1; }
		virtual std::any createAnyPointer(uintptr_t pointer) { return nullptr; }
		virtual std::any getKeyAtIndex(std::any container, int index) const { return std::any(); }
		virtual CatGenericType getKeyType() const { return keyType; }
		virtual CatGenericType getValueType() const { return valueType; }
	private:
		CatGenericType keyType;
		CatGenericType valueType;
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


		virtual std::any getItemAt(std::any container, std::any key) override final
		{
			return getItemAt(container, std::any_cast<int>(key));
		}


		virtual int getIndexOf(std::any container, std::any key) override final
		{
			return -1;
		}
		

		virtual std::any createAnyPointer(uintptr_t pointer) override final
		{
			return std::any(reinterpret_cast<VectorT*>(pointer));
		}


		virtual std::any getKeyAtIndex(std::any container, int index) const override final
		{
			return std::any();
		}


		virtual CatGenericType getKeyType() const override final
		{
			return CatGenericType::intType;
		}


		virtual CatGenericType getValueType() const override final
		{
			return TypeTraits<typename VectorT::value_type>::toGenericType();
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


		virtual std::any getItemAt(std::any container, std::any key) override final
		{
			MapT* map = std::any_cast<MapT*>(container);
			if (map != nullptr)
			{
				if constexpr (std::is_same<typename MapT::key_type, std::string>::value)
				{
					std::string lowerKey = Tools::toLowerCase(std::any_cast<std::string>(key));
					auto& iter = map->find(lowerKey);
					if (iter != map->end())
					{
						return TypeTraits<typename MapT::mapped_type>::getCatValue(iter->second);
					}
				}
				else
				{
					auto& iter = map->find(std::any_cast<typename MapT::key_type>(key));
					if (iter != map->end())
					{
						return TypeTraits<typename MapT::mapped_type>::getCatValue(iter->second);
					}
				}
			}
			return TypeTraits<typename MapT::mapped_type>::getDefaultCatValue();
		}


		virtual int getIndexOf(std::any container, std::any key) override final
		{
			MapT* map = std::any_cast<MapT*>(container);
			if (map != nullptr)
			{
				if constexpr (std::is_same<typename MapT::key_type, std::string>::value)
				{
					std::string lowerIndex = Tools::toLowerCase(std::any_cast<std::string>(key));
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
				else
				{
					int count = 0;
					for (auto& iter : *map)
					{
						if (iter.first == std::any_cast<typename MapT::key_type>(key))
						{
							return count;
						}
						count++;
					}
				}
			}
			return -1;
		}


		virtual std::any getKeyAtIndex(std::any container, int index) const override final
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
			return TypeTraits<typename MapT::key_type>::getDefaultCatValue();
		}


		virtual std::any createAnyPointer(uintptr_t pointer) override final
		{
			return std::any(reinterpret_cast<MapT*>(pointer));
		}


		virtual CatGenericType getKeyType() const override final
		{
			return TypeTraits<typename MapT::key_type>::toGenericType();
		}


		virtual CatGenericType getValueType() const override final
		{
			return TypeTraits<typename MapT::mapped_type>::toGenericType();
		}
	};

}