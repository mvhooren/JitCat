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
	struct MemberFunctionInfo;

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
		virtual const CatGenericType& getKeyType() const = 0;
		virtual const CatGenericType& getValueType() const = 0;
		virtual MemberFunctionInfo* getMemberFunctionInfo(const std::string& functionName) = 0;
	};								


	//Used for deserialized type information, not backed by actual reflected type.
	class DummyManipulator: public ContainerManipulator
	{
	public:
		DummyManipulator(const CatGenericType& keyType, const CatGenericType& valueType): keyType(keyType), valueType(valueType) {}
		virtual std::size_t getContainerSize(std::any container) const override final {return 0;}
		virtual std::any getItemAt(std::any container, int index) override final {	return std::any();}
		virtual std::any getItemAt(std::any container, std::any key) override final { return std::any(); }
		virtual int getIndexOf(std::any container, std::any key) override final { return -1; }
		virtual std::any createAnyPointer(uintptr_t pointer) override final { return nullptr; }
		virtual std::any getKeyAtIndex(std::any container, int index) const override final  { return std::any(); }
		virtual const CatGenericType& getKeyType() const override final { return keyType; }
		virtual const CatGenericType& getValueType() const override final { return valueType; }
		virtual MemberFunctionInfo* getMemberFunctionInfo(const std::string& functionName) override final {return nullptr;}
	private:
		CatGenericType keyType;
		CatGenericType valueType;
	};

	struct ArrayTypeMemberFunctionInfo;

	class ArrayManipulator: public ContainerManipulator
	{
	public:
		struct Array: public Reflectable
		{
			Array():
				arrayData(nullptr),
				size(0),
				reserved(0)
			{}
			unsigned char* arrayData;
			//current size in number of items (not bytes)
			int size;
			//reserved size in number of items (not bytes)
			int reserved;
		};
	private:
		ArrayManipulator(CatGenericType valueType);
	public:
		virtual std::size_t getContainerSize(std::any container) const override final;
		virtual std::any getItemAt(std::any container, int index) override final;
		virtual std::any getItemAt(std::any container, std::any key) override final;
		virtual int getIndexOf(std::any container, std::any key) override final;
		virtual std::any createAnyPointer(uintptr_t pointer) override final;
		virtual std::any getKeyAtIndex(std::any container, int index) const override final;
		virtual const CatGenericType& getKeyType() const override final;
		virtual const CatGenericType& getValueType() const override final;
		virtual MemberFunctionInfo* getMemberFunctionInfo(const std::string& functionName) override final;

		int add(Array* array, const std::any& value);
		void remove(Array* array, int index);

		static void placementConstruct(unsigned char* buffer, std::size_t bufferSize);
		void placementDestruct(unsigned char* buffer, std::size_t bufferSize);
		void copy(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize);
		void move(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize);

		static ArrayManipulator& createArrayManipulatorOf(CatGenericType valueType);
		static void deleteArrayManipulatorsOfType(TypeInfo* objectType);

		ArrayTypeMemberFunctionInfo* getAddFunction();
		ArrayTypeMemberFunctionInfo* getRemoveFunction();
		ArrayTypeMemberFunctionInfo* getSizeFunction();

	private:
		CatGenericType valueType;

		std::unique_ptr<ArrayTypeMemberFunctionInfo> addFunction;
		std::unique_ptr<ArrayTypeMemberFunctionInfo> removeFunction;
		std::unique_ptr<ArrayTypeMemberFunctionInfo> sizeFunction;

		static std::vector<std::unique_ptr<ArrayManipulator>> manipulators;

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


		virtual const CatGenericType& getKeyType() const override final
		{
			return CatGenericType::intType;
		}


		virtual const CatGenericType& getValueType() const override final
		{
			return TypeTraits<typename VectorT::value_type>::toGenericType();
		}

		virtual MemberFunctionInfo* getMemberFunctionInfo(const std::string& functionName) override final {return nullptr;}
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


		virtual const CatGenericType& getKeyType() const override final
		{
			return TypeTraits<typename MapT::key_type>::toGenericType();
		}


		virtual const CatGenericType& getValueType() const override final
		{
			return TypeTraits<typename MapT::mapped_type>::toGenericType();
		}

		virtual MemberFunctionInfo* getMemberFunctionInfo(const std::string& functionName) override final {return nullptr;}
	};

}