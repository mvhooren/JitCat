/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

//This file contains external reflectors for various std library containers.
#pragma once

#include "jitcat/Configuration.h"
#include "jitcat/ExternalReflector.h"
#include "jitcat/TypeTools.h"
#include "jitcat/Tools.h"

#include <cassert>
#include <array>
#include <deque>
#include <map>
#include <memory>
#include <sstream>
#include <string>
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


		template <typename ReturnT>
		inline ReturnT getDefault()
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
			static inline const char* getTypeName();
			static inline void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
			static inline typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType safeIndex(VectorT* vector, int key);
			static inline int size(VectorT* vector);

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
			static inline const char* getTypeName();
			static inline void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
			static inline bool safeIndex(VectorT* vector, int key);
			static inline int size(VectorT* vector);
			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = true;
	};


	//Reflection for std::array
	template <class ItemT, std::size_t ArraySize>
	class ExternalReflector<std::array<ItemT, ArraySize>>
	{
		using ArrayT = std::array<ItemT, ArraySize>;
		public:
			static inline const char* getTypeName();
			static inline void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
			static inline typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType safeIndex(ArrayT* array, int key);
			static inline int size(ArrayT* array);

			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = TypeTools::getAllowCopyConstruction<ItemT>();
	};



	//Reflection for std::array
	template <class ItemT, class AllocatorT>
	class ExternalReflector<std::deque<ItemT, AllocatorT>>
	{
		using DequeT = std::deque<ItemT, AllocatorT>;
		public:
			static inline const char* getTypeName();
			static inline void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
			static inline typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType safeIndex(DequeT* deque, int key);
			static inline int size(DequeT* deque);

			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = TypeTools::getAllowCopyConstruction<ItemT>();
	};


	//Reflection for std::map
	template <class KeyT, class ValueT, class PredicateT, class AllocatorT>
	class ExternalReflector<std::map<KeyT, ValueT, PredicateT, AllocatorT>>
	{
		using MapT = std::map<KeyT, ValueT, PredicateT, AllocatorT>;
		public:
			static inline const char* getTypeName();
			static inline void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
			static inline typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType safeIndex(MapT* map, const KeyT& key);
			static inline typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType ordinalIndex(MapT* map, int ordinal);
			static inline int size(MapT* map);

			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = TypeTools::getAllowCopyConstruction<KeyT>() && TypeTools::getAllowCopyConstruction<ValueT>();
	}; 


	//Reflection for std::unordered_map
	template <class KeyT, class ValueT, class HashT, class PredicateT, class AllocatorT>
	class ExternalReflector<std::unordered_map<KeyT, ValueT, HashT, PredicateT, AllocatorT>>
	{
		using MapT = std::unordered_map<KeyT, ValueT, HashT, PredicateT, AllocatorT>;
		public:
			static inline const char* getTypeName();
			static inline void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);
			static inline typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType safeIndex(MapT* map, const KeyT& key);
			static inline typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType ordinalIndex(MapT* map, int ordinal);
			static inline int size(MapT* map);

			static constexpr bool exists = true;
			static constexpr bool enableCopyConstruction = TypeTools::getAllowCopyConstruction<KeyT>() && TypeTools::getAllowCopyConstruction<ValueT>();
	}; 


	//Reflection for std::string
	template <class CharT, class TraitsT, class AllocatorT>
	class ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>
	{
		using StringT = std::basic_string<CharT, TraitsT, AllocatorT>;
		using StringStreamT = std::basic_stringstream<CharT, TraitsT, AllocatorT>;

		public:
			static inline const char* getTypeName();
			static inline void reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo);

			static constexpr bool exists = true;

		private:

			static inline int length(StringT* string);
			static inline StringT calculateSimpleStringAddition(const StringT* lString, const StringT* rString);
			static inline bool stringEquals(const StringT* lString, const StringT* rString);
			static inline bool stringNotEquals(const StringT* lString, const StringT* rString);
			static inline int find(StringT* thisString, const StringT* stringToFind);
			static inline int find(StringT* thisString, const StringT* stringToFind, int offset);
			static inline StringT replace(StringT* thisString, const StringT* stringToFind, const StringT* replacementString);
			static inline StringT subString(StringT* thisString, int start, int length);
			template<typename LeftT, typename RightT>
			static inline StringT calculateStringAddition(LeftT lValue, RightT rValue);
	};

}


#include "jitcat/STLTypeReflectorsHeaderImplementation.h"
