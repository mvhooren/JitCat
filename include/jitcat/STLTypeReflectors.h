/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

//This file contains external reflectors for various std library containers.
#pragma once

#include "jitcat/ExternalReflector.h"
#include "jitcat/TypeTools.h"

#include <cassert>
#include <array>
#include <deque>
#include <map>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>



namespace jitcat::Reflection
{

	namespace STLHelper
	{
		template <typename ItemT>
		class ValueReturnType
		{
		private:
			ValueReturnType() = delete;
			~ValueReturnType() = delete;
			ValueReturnType(const ValueReturnType&) = delete;
		public:
			typedef ItemT* containerItemReturnType;
		};

		template <typename ItemT>
		class ValueReturnType<std::unique_ptr<ItemT>>
		{
		private:
			ValueReturnType() = delete;
			~ValueReturnType() = delete;
			ValueReturnType(const ValueReturnType&) = delete;
		public:
			typedef ItemT* containerItemReturnType;
		};


		template <typename ItemT>
		inline typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType getValue(ItemT& itemValue)
		{
			if constexpr (TypeTraits<ItemT>::isUniquePtr())
			{
				return itemValue.get();
			}
			else
			{
				return &itemValue;
			}
		}


		template <typename ReturnT>
		inline typename ReturnT getDefault()
		{
			if constexpr (std::is_pointer_v<ReturnT>)
			{
				static ReturnT value = nullptr;
				value = nullptr;
				return value;
			}
			else if constexpr (std::is_default_constructible_v<ReturnT>)
			{
				static std::unique_ptr<ReturnT> defaultValue;
				defaultValue = std::make_unique<ReturnT>();
				return *defaultValue.get();
			}
			else
			{
				assert(false);
				abort();
			}
		}
	};

	//Reflection for std::vector
	template <class ItemT, class AllocatorT>
	class ExternalReflector<std::vector<ItemT, AllocatorT>>
	{
		using VectorT = std::vector<ItemT, AllocatorT>;
		public:
			static const char* getTypeName()
			{
				static const std::string vectorName = jitcat::Tools::append("vector<", TypeNameGetter<ItemT>::get() , 
																			",", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
				return vectorName.c_str();
			}


			static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
			{
				typeInfo
					.addPseudoMemberFunction<VectorT>("[]", &ExternalReflector<VectorT>::safeIndex)
					.addPseudoMemberFunction<VectorT>("size", &ExternalReflector<VectorT>::size);
			}


			static typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType safeIndex(VectorT* vector, int key)
			{
				if (vector != nullptr && key >= 0 && key < (int)vector->size())
				{
					return STLHelper::getValue(vector->operator[](key));
				}
				return STLHelper::getDefault<STLHelper::ValueReturnType<ItemT>::containerItemReturnType>();
			}


			static int size(VectorT* vector)
			{
				if (vector != nullptr)
				{
					return (int)vector->size();
				}
				return 0;
			}
			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = TypeTools::getAllowCopyConstruction<ItemT>();
	};


	//Reflection for std::vector of bool
	//A vector of booleans returns a weird reference type. (Because a vector of booleans is compressed to a bitfield).
	//Therefore, we cannot return a pointer to the item from operator[]. Hence this specialization.
	template <class AllocatorT>
	class ExternalReflector<std::vector<bool, AllocatorT>>
	{
		using VectorT = std::vector<bool, AllocatorT>;
		public:
			static const char* getTypeName()
			{
				static const std::string vectorName = jitcat::Tools::append("vector<bool,", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
				return vectorName.c_str();
			}


			static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
			{
				typeInfo
					.addPseudoMemberFunction<VectorT>("[]", &ExternalReflector<VectorT>::safeIndex)
					.addPseudoMemberFunction<VectorT>("size", &ExternalReflector<VectorT>::size);
			}


			static typename bool safeIndex(VectorT* vector, int key)
			{
				if (vector != nullptr && key >= 0 && key < (int)vector->size())
				{
					return vector->operator[](key);
				}
				return STLHelper::getDefault<bool>();
			}


			static int size(VectorT* vector)
			{
				if (vector != nullptr)
				{
					return (int)vector->size();
				}
				return 0;
			}
			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = true;
	};


	//Reflection for std::array
	template <class ItemT, int ArraySize>
	class ExternalReflector<std::array<ItemT, ArraySize>>
	{
		using ArrayT = std::array<ItemT, ArraySize>;
		public:
			static const char* getTypeName()
			{
				static const std::string arrayName = jitcat::Tools::append("array<", TypeNameGetter<ItemT>::get(), 
																		   ",", ArraySize, ">");
				return arrayName.c_str();
			}


			static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
			{
				typeInfo
					.addPseudoMemberFunction<ArrayT>("[]", &ExternalReflector<ArrayT>::safeIndex)
					.addPseudoMemberFunction<ArrayT>("size", &ExternalReflector<ArrayT>::size);
			}


			static typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType safeIndex(ArrayT* array, int key)
			{
				if (array != nullptr && key >= 0 && key < ArraySize)
				{
					return STLHelper::getValue(array->operator[](key));
				}
				return STLHelper::getDefault<STLHelper::ValueReturnType<ItemT>::containerItemReturnType>();
			}


			static int size(ArrayT* array)
			{
				if (array != nullptr)
				{
					return (int)array->size();
				}
				return 0;
			}

			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = TypeTools::getAllowCopyConstruction<ItemT>();
	};



	//Reflection for std::array
	template <class ItemT, class AllocatorT>
	class ExternalReflector<std::deque<ItemT, AllocatorT>>
	{
		using DequeT = std::deque<ItemT, AllocatorT>;
		public:
			static const char* getTypeName()
			{
				static const std::string dequeName = jitcat::Tools::append("deque<", TypeNameGetter<ItemT>::get(), 
																		   ",", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
				return dequeName.c_str();
			}


			static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
			{
				typeInfo
					.addPseudoMemberFunction<DequeT>("[]", &ExternalReflector<DequeT>::safeIndex)
					.addPseudoMemberFunction<DequeT>("size", &ExternalReflector<DequeT>::size);
			}


			static typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType safeIndex(DequeT* deque, int key)
			{
				if (deque != nullptr && key >= 0 && key < (int)deque->size())
				{
					return STLHelper::getValue(deque->operator[](key));
				}
				return STLHelper::getDefault<STLHelper::ValueReturnType<ItemT>::containerItemReturnType>();
			}


			static int size(DequeT* deque)
			{
				if (deque != nullptr)
				{
					return (int)deque->size();
				}
				return 0;
			}

			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = TypeTools::getAllowCopyConstruction<ItemT>();
	};


	//Reflection for std::map
	template <class KeyT, class ValueT, class PredicateT, class AllocatorT>
	class ExternalReflector<std::map<KeyT, ValueT, PredicateT, AllocatorT>>
	{
		using MapT = std::map<KeyT, ValueT, PredicateT, AllocatorT>;
		public:
			static const char* getTypeName()
			{
				static const std::string unorderedMapName = jitcat::Tools::append("map<", TypeNameGetter<KeyT>::get() , 
																				  ",", TypeNameGetter<ValueT>::get(), 
																				  ",", TypeIdentifier<PredicateT>::getIdentifier(), 
																				  ",", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
				return unorderedMapName.c_str();
			}


			static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
			{
				typeInfo
					.addPseudoMemberFunction<MapT>("[]", &ExternalReflector<MapT>::safeIndex)
					.addPseudoMemberFunction<MapT>("size", &ExternalReflector<MapT>::size)
					.addPseudoMemberFunction<MapT>("index", &ExternalReflector<MapT>::ordinalIndex);
			}


			static typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType safeIndex(MapT* map, const KeyT& key)
			{
				if (map != nullptr)
				{
					auto& iter = map->find(key);
					if (iter != map->end())
					{
						return STLHelper::getValue(iter->second);
					}
				}
				return STLHelper::getDefault<STLHelper::ValueReturnType<ValueT>::containerItemReturnType>();
			}


			static typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType ordinalIndex(MapT* map, int ordinal)
			{
				if (map != nullptr)
				{
					int counter = 0;
					for (auto& iter = map->begin(); iter != map->end(); ++iter)
					{
						if (counter == ordinal)
						{
							return STLHelper::getValue(iter->second);
						}
						counter++;
					}
				}
				return STLHelper::getDefault<STLHelper::ValueReturnType<ValueT>::containerItemReturnType>();
			}


			static int size(MapT* map)
			{
				if (map != nullptr)
				{
					return (int)map->size();
				}
				return 0;
			}
			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = TypeTools::getAllowCopyConstruction<KeyT>() && TypeTools::getAllowCopyConstruction<ValueT>();
	}; 


	//Reflection for std::unordered_map
	template <class KeyT, class ValueT, class HashT, class PredicateT, class AllocatorT>
	class ExternalReflector<std::unordered_map<KeyT, ValueT, HashT, PredicateT, AllocatorT>>
	{
		using MapT = std::unordered_map<KeyT, ValueT, HashT, PredicateT, AllocatorT>;
		public:
			static const char* getTypeName()
			{
				static const std::string unorderedMapName = jitcat::Tools::append("unordered_map<", TypeNameGetter<KeyT>::get() , 
																				  ",", TypeNameGetter<ValueT>::get(), 
																				  ",", TypeIdentifier<HashT>::getIdentifier(), 
																				  ",", TypeIdentifier<PredicateT>::getIdentifier(), 
																				  ",", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
				return unorderedMapName.c_str();
			}


			static void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
			{
				typeInfo
					.addPseudoMemberFunction<MapT>("[]", &ExternalReflector<MapT>::safeIndex)
					.addPseudoMemberFunction<MapT>("size", &ExternalReflector<MapT>::size)
					.addPseudoMemberFunction<MapT>("index", &ExternalReflector<MapT>::ordinalIndex);
			}


			static typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType safeIndex(MapT* map, const KeyT& key)
			{
				if (map != nullptr)
				{
					auto& iter = map->find(key);
					if (iter != map->end())
					{
						return STLHelper::getValue(iter->second);
					}
				}
				return STLHelper::getDefault<STLHelper::ValueReturnType<ValueT>::containerItemReturnType>();
			}


			static typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType ordinalIndex(MapT* map, int ordinal)
			{
				if (map != nullptr)
				{
					int counter = 0;
					for (auto& iter = map->begin(); iter != map->end(); ++iter)
					{
						if (counter == ordinal)
						{
							return STLHelper::getValue(iter->second);
						}
						counter++;
					}
				}
				return STLHelper::getDefault<STLHelper::ValueReturnType<ValueT>::containerItemReturnType>();
			}


			static int size(MapT* map)
			{
				if (map != nullptr)
				{
					return (int)map->size();
				}
				return 0;
			}

			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = TypeTools::getAllowCopyConstruction<KeyT>() && TypeTools::getAllowCopyConstruction<ValueT>();
	}; 

}