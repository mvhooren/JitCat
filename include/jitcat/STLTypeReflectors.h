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


		template <>
		class ValueReturnType<bool>
		{
		private:
			ValueReturnType() = delete;
			~ValueReturnType() = delete;
			ValueReturnType(const ValueReturnType&) = delete;
		public:
			typedef bool containerItemReturnType;
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
			//using ItemT = std::remove_reference_t<typename STLHelper::ValueReturnType<ReturnT>::containerItemReturnType>;
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
					if constexpr (std::is_same_v<bool, ItemT>)
					{
						//A vector of booleans returns a weird reference type. (Because a vector of booleans is compressed to a bitfield).
						//Therefore, we cannot return a pointer to it.
						return vector->operator[](key);
					}
					else
					{
						return STLHelper::getValue(vector->operator[](key));
					}
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