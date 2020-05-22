/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/TypeTraits.h"


namespace jitcat::Reflection
{
	namespace STLHelper
	{
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
	}


	template <class ItemT, class AllocatorT>
	inline const char* ExternalReflector<std::vector<ItemT, AllocatorT>>::getTypeName()
	{
		static const std::string vectorName = jitcat::Tools::append("vector<", TypeNameGetter<ItemT>::get() , 
																	",", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
		return vectorName.c_str();
	}


	template <class ItemT, class AllocatorT>
	inline void ExternalReflector<std::vector<ItemT, AllocatorT>>::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
	{
		typeInfo
			.template addPseudoMemberFunction<VectorT>("[]", &ExternalReflector<VectorT>::safeIndex)
			.template addPseudoMemberFunction<VectorT>("size", &ExternalReflector<VectorT>::size);
	}


	template <class ItemT, class AllocatorT>
	inline typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType ExternalReflector<std::vector<ItemT, AllocatorT>>::safeIndex(VectorT* vector, int key)
	{
		if (vector != nullptr && key >= 0 && key < (int)vector->size())
		{
			return STLHelper::getValue(vector->operator[](key));
		}
		return STLHelper::getDefault<typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType>();
	}


	template <class ItemT, class AllocatorT>
	inline int ExternalReflector<std::vector<ItemT, AllocatorT>>::size(VectorT* vector)
	{
		if (vector != nullptr)
		{
			return (int)vector->size();
		}
		return 0;
	}


	template <class AllocatorT>
	inline const char*  ExternalReflector<std::vector<bool, AllocatorT>>::getTypeName()
	{
		static const std::string vectorName = jitcat::Tools::append("vector<bool,", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
		return vectorName.c_str();
	}


	template <class AllocatorT>
	inline void ExternalReflector<std::vector<bool, AllocatorT>>::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
	{
		typeInfo
			.template addPseudoMemberFunction<VectorT>("[]", &ExternalReflector<VectorT>::safeIndex)
			.template addPseudoMemberFunction<VectorT>("size", &ExternalReflector<VectorT>::size);
	}


	template<class AllocatorT>
	inline bool ExternalReflector<std::vector<bool, AllocatorT>>::safeIndex(VectorT* vector, int key)
	{
		if (vector != nullptr && key >= 0 && key < (int)vector->size())
		{
			return vector->operator[](key);
		}
		return STLHelper::getDefault<bool>();
	}


	template<class AllocatorT>
	inline int ExternalReflector<std::vector<bool, AllocatorT>>::size(VectorT* vector)
	{
		if (vector != nullptr)
		{
			return (int)vector->size();
		}
		return 0;
	}


	template<class ItemT, std::size_t ArraySize>
	inline const char* ExternalReflector<std::array<ItemT, ArraySize>>::getTypeName()
	{
		static const std::string arrayName = jitcat::Tools::append("array<", TypeNameGetter<ItemT>::get(),
																   ",", ArraySize, ">");
		return arrayName.c_str();
	}


	template<class ItemT, std::size_t ArraySize>
	inline void ExternalReflector<std::array<ItemT, ArraySize>>::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
	{
		typeInfo
			.template addPseudoMemberFunction<ArrayT>("[]", &ExternalReflector<ArrayT>::safeIndex)
			.template addPseudoMemberFunction<ArrayT>("size", &ExternalReflector<ArrayT>::size);
	}


	template<class ItemT, std::size_t ArraySize>
	inline typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType ExternalReflector<std::array<ItemT, ArraySize>>::safeIndex(ArrayT* array, int key)
	{
		if (array != nullptr && key >= 0 && key < ArraySize)
		{
			return STLHelper::getValue(array->operator[](key));
		}
		return STLHelper::getDefault<typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType>();
	}


	template<class ItemT, std::size_t ArraySize>
	inline int ExternalReflector<std::array<ItemT, ArraySize>>::size(ArrayT* array)
	{
		if (array != nullptr)
		{
			return (int)array->size();
		}
		return 0;
	}


	template<class ItemT, class AllocatorT>
	inline const char* ExternalReflector<std::deque<ItemT, AllocatorT>>::getTypeName()
	{
		static const std::string dequeName = jitcat::Tools::append("deque<", TypeNameGetter<ItemT>::get(),
			",", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
		return dequeName.c_str();
	}


	template<class ItemT, class AllocatorT>
	inline void ExternalReflector<std::deque<ItemT, AllocatorT>>::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
	{
		typeInfo
			.template addPseudoMemberFunction<DequeT>("[]", &ExternalReflector<DequeT>::safeIndex)
			.template addPseudoMemberFunction<DequeT>("size", &ExternalReflector<DequeT>::size);
	}


	template<class ItemT, class AllocatorT>
	inline typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType ExternalReflector<std::deque<ItemT, AllocatorT>>::safeIndex(DequeT* deque, int key)
	{
		if (deque != nullptr && key >= 0 && key < (int)deque->size())
		{
			return STLHelper::getValue(deque->operator[](key));
		}
		return STLHelper::getDefault<typename STLHelper::ValueReturnType<ItemT>::containerItemReturnType>();
	}


	template<class ItemT, class AllocatorT>
	inline int ExternalReflector<std::deque<ItemT, AllocatorT>>::size(DequeT* deque)
	{
		if (deque != nullptr)
		{
			return (int)deque->size();
		}
		return 0;
	}


	template<class KeyT, class ValueT, class PredicateT, class AllocatorT>
	inline const char* ExternalReflector<std::map<KeyT, ValueT, PredicateT, AllocatorT>>::getTypeName()
	{
		static const std::string unorderedMapName = jitcat::Tools::append("map<", TypeNameGetter<KeyT>::get(),
			",", TypeNameGetter<ValueT>::get(),
			",", TypeIdentifier<PredicateT>::getIdentifier(),
			",", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
		return unorderedMapName.c_str();
	}


	template<class KeyT, class ValueT, class PredicateT, class AllocatorT>
	inline void ExternalReflector<std::map<KeyT, ValueT, PredicateT, AllocatorT>>::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
	{
		typeInfo
			.template addPseudoMemberFunction<MapT>("[]", &ExternalReflector<MapT>::safeIndex)
			.template addPseudoMemberFunction<MapT>("size", &ExternalReflector<MapT>::size)
			.template addPseudoMemberFunction<MapT>("index", &ExternalReflector<MapT>::ordinalIndex);
	}


	template<class KeyT, class ValueT, class PredicateT, class AllocatorT>
	inline typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType ExternalReflector<std::map<KeyT, ValueT, PredicateT, AllocatorT>>::safeIndex(MapT* map, const KeyT& key)
	{
		if (map != nullptr)
		{
			auto iter = map->find(key);
			if (iter != map->end())
			{
				return STLHelper::getValue(iter->second);
			}
		}
		return STLHelper::getDefault<typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType>();
	}


	template<class KeyT, class ValueT, class PredicateT, class AllocatorT>
	inline typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType ExternalReflector<std::map<KeyT, ValueT, PredicateT, AllocatorT>>::ordinalIndex(MapT* map, int ordinal)
	{
		if (map != nullptr)
		{
			int counter = 0;
			for (auto iter = map->begin(); iter != map->end(); ++iter)
			{
				if (counter == ordinal)
				{
					return STLHelper::getValue(iter->second);
				}
				counter++;
			}
		}
		return STLHelper::getDefault<typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType>();
	}


	template<class KeyT, class ValueT, class PredicateT, class AllocatorT>
	inline int ExternalReflector<std::map<KeyT, ValueT, PredicateT, AllocatorT>>::size(MapT* map)
	{
		if (map != nullptr)
		{
			return (int)map->size();
		}
		return 0;
	}


	template<class KeyT, class ValueT, class HashT, class PredicateT, class AllocatorT>
	inline const char* ExternalReflector<std::unordered_map<KeyT, ValueT, HashT, PredicateT, AllocatorT>>::getTypeName()
	{
		static const std::string unorderedMapName = jitcat::Tools::append("unordered_map<", TypeNameGetter<KeyT>::get(),
			",", TypeNameGetter<ValueT>::get(),
			",", TypeIdentifier<HashT>::getIdentifier(),
			",", TypeIdentifier<PredicateT>::getIdentifier(),
			",", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
		return unorderedMapName.c_str();
	}


	template<class KeyT, class ValueT, class HashT, class PredicateT, class AllocatorT>
	inline void ExternalReflector<std::unordered_map<KeyT, ValueT, HashT, PredicateT, AllocatorT>>::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
	{
		typeInfo
			.template addPseudoMemberFunction<MapT>("[]", &ExternalReflector<MapT>::safeIndex)
			.template addPseudoMemberFunction<MapT>("size", &ExternalReflector<MapT>::size)
			.template addPseudoMemberFunction<MapT>("index", &ExternalReflector<MapT>::ordinalIndex);
	}


	template<class KeyT, class ValueT, class HashT, class PredicateT, class AllocatorT>
	inline typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType ExternalReflector<std::unordered_map<KeyT, ValueT, HashT, PredicateT, AllocatorT>>::safeIndex(MapT* map, const KeyT& key)
	{
		if (map != nullptr)
		{
			auto iter = map->find(key);
			if (iter != map->end())
			{
				return STLHelper::getValue(iter->second);
			}
		}
		return STLHelper::getDefault<typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType>();
	}


	template<class KeyT, class ValueT, class HashT, class PredicateT, class AllocatorT>
	inline typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType ExternalReflector<std::unordered_map<KeyT, ValueT, HashT, PredicateT, AllocatorT>>::ordinalIndex(MapT* map, int ordinal)
	{
		if (map != nullptr)
		{
			int counter = 0;
			for (auto iter = map->begin(); iter != map->end(); ++iter)
			{
				if (counter == ordinal)
				{
					return STLHelper::getValue(iter->second);
				}
				counter++;
			}
		}
		return STLHelper::getDefault<typename STLHelper::ValueReturnType<ValueT>::containerItemReturnType>();
	}


	template<class KeyT, class ValueT, class HashT, class PredicateT, class AllocatorT>
	inline int ExternalReflector<std::unordered_map<KeyT, ValueT, HashT, PredicateT, AllocatorT>>::size(MapT* map)
	{
		if (map != nullptr)
		{
			return (int)map->size();
		}
		return 0;
	}


	template<class CharT, class TraitsT, class AllocatorT>
	inline const char* ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::getTypeName()
	{
		if constexpr (std::is_same_v<StringT, Configuration::CatString>)
		{
			return "string";
		}
		else
		{
			static const std::string stringName = jitcat::Tools::append("string<", TypeNameGetter<CharT>::get(),
				",", TypeIdentifier<TraitsT>::getIdentifier(),
				",", TypeIdentifier<AllocatorT>::getIdentifier(), ">");
			return stringName.c_str();
		}
	}


	template<class CharT, class TraitsT, class AllocatorT>
	inline void ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
	{
		typeInfo
			.addMember("+", &ExternalReflector<StringT>::calculateSimpleStringAddition)
			.addMember("+", &ExternalReflector<StringT>::calculateStringAddition<int, const StringT*>)
			.addMember("+", &ExternalReflector<StringT>::calculateStringAddition<const StringT*, int>)
			.addMember("+", &ExternalReflector<StringT>::calculateStringAddition<float, const StringT*>)
			.addMember("+", &ExternalReflector<StringT>::calculateStringAddition<const StringT*, float>)
			.addMember("+", &ExternalReflector<StringT>::calculateStringAddition<bool, const StringT*>)
			.addMember("+", &ExternalReflector<StringT>::calculateStringAddition<const StringT*, bool>)
			.addMember("+", &ExternalReflector<StringT>::calculateStringAddition<const StringT*, double>)
			.addMember("+", &ExternalReflector<StringT>::calculateStringAddition<double, const StringT*>)
			.addMember("==", &ExternalReflector<StringT>::stringEquals)
			.addMember("!=", &ExternalReflector<StringT>::stringNotEquals)
			.template addMember<StringT, StringT&, const StringT&>("=", &StringT::operator=)
			.template addPseudoMemberFunction<StringT>("length", &ExternalReflector<StringT>::length)
			.template addPseudoMemberFunction<StringT, int, const StringT*>("find", &ExternalReflector<StringT>::find)
			.template addPseudoMemberFunction<StringT, int, const StringT*, int>("find", &ExternalReflector<StringT>::find)
			.template addPseudoMemberFunction<StringT>("replace", &ExternalReflector<StringT>::replace)
			.template addPseudoMemberFunction<StringT>("subString", &ExternalReflector<StringT>::subString);
	}


	template<class CharT, class TraitsT, class AllocatorT>
	inline int ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::length(StringT* string)
	{
		if (string != nullptr)
		{
			return (int)string->length();
		}
		else
		{
			return 0;
		}
	}


	template<class CharT, class TraitsT, class AllocatorT>
	inline std::basic_string<CharT, TraitsT, AllocatorT> ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::calculateSimpleStringAddition(const StringT* lString, const StringT* rString)
	{
		if (lString != nullptr && rString != nullptr)	return *lString + *rString;
		else if (lString != nullptr)					return *lString;
		else if (rString != nullptr)					return *rString;
		else											return StringT();
	}


	template<class CharT, class TraitsT, class AllocatorT>
	inline bool ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::stringEquals(const StringT* lString, const StringT* rString)
	{
		if (lString != nullptr && rString != nullptr)
		{
			return *lString == *rString;
		}
		return false;
	}


	template<class CharT, class TraitsT, class AllocatorT>
	inline bool ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::stringNotEquals(const StringT* lString, const StringT* rString)
	{
		if (lString != nullptr && rString != nullptr)
		{
			return *lString != *rString;
		}
		return false;
	}


	template<class CharT, class TraitsT, class AllocatorT>
	inline int ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::find(StringT* thisString, const StringT* stringToFind)
	{
		if (thisString != nullptr && stringToFind != nullptr)
		{
			return (int)thisString->find(*stringToFind);
		}
		else
		{
			return -1;
		}
	}


	template<class CharT, class TraitsT, class AllocatorT>
	inline int ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::find(StringT* thisString, const StringT* stringToFind, int offset)
	{
		if (thisString != nullptr && stringToFind != nullptr && offset >= 0)
		{
			return (int)thisString->find(*stringToFind, (std::size_t)offset);
		}
		else
		{
			return -1;
		}
	}


	template<class CharT, class TraitsT, class AllocatorT>
	inline std::basic_string<CharT, TraitsT, AllocatorT> ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::replace(StringT* thisString, const StringT* stringToFind, const StringT* replacementString)
	{
		if (thisString == nullptr || stringToFind == nullptr || replacementString == nullptr)
		{
			return StringT();
		}
		if (*thisString != StringT())
		{
			StringT newString = *thisString;
			size_t startPosition = 0;
			while ((startPosition = newString.find(*stringToFind, startPosition)) != StringT::npos)
			{
				newString.replace(startPosition, stringToFind->length(), *replacementString);
				startPosition += replacementString->length();
			}
			return newString;
		}
		return *thisString;
	}


	template<class CharT, class TraitsT, class AllocatorT>
	inline std::basic_string<CharT, TraitsT, AllocatorT> ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::subString(StringT* thisString, int start, int length)
	{
		if (thisString == nullptr || thisString->size() == 0)
		{
			return StringT();
		}
		else if ((int)thisString->size() > start && start >= 0)
		{
			return thisString->substr((unsigned int)start, (unsigned int)length);
		}
		else
		{
			return StringT();
		}
	}


	template<class CharT, class TraitsT, class AllocatorT>
	template<typename LeftT, typename RightT>
	inline std::basic_string<CharT, TraitsT, AllocatorT> ExternalReflector<std::basic_string<CharT, TraitsT, AllocatorT>>::calculateStringAddition(LeftT lValue, RightT rValue)
	{
		StringStreamT stream = StringStreamT();
		if constexpr (std::is_same_v<LeftT, const StringT*>)
		{
			if (lValue != nullptr)	stream << *lValue;
		}
		else
		{
			stream << lValue;
		}

		if constexpr (std::is_same_v<RightT, const StringT*>)
		{
			if (rValue != nullptr)	stream << *rValue;
		}
		else
		{
			stream << rValue;
		}
		return stream.str();
	}
}
