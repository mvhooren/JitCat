/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "Tools.h"

#include <sstream>
#include <string.h>


template<typename T>
inline void Tools::deleteElements(std::vector<T>& vector)
{
	for (auto& i : vector)
	{
		delete i;
	}
	vector.clear();
}


template<typename ContainerType>
inline void Tools::deleteSecondElementsAndClear(ContainerType& map)
{
	typename ContainerType::const_iterator end = map.end();
	for (typename ContainerType::iterator iter = map.begin(); iter != end; ++iter)
	{
		delete (*iter).second;
	}
	map.clear();
}


template<typename T1, typename T2>
inline bool Tools::isInList(const std::vector<T1>& vector, const T2 & element)
{
	std::size_t size = vector.size();
	for (std::size_t i = 0; i < size; ++i)
	{
		if (vector[i] == element)
		{
			return true;
		}
	}
	return false;
}


template<typename T>
inline T Tools::convert(const std::string& text)
{
	return convert<T>(text.c_str());
}


template<>
inline bool Tools::convert<bool>(const char* text)
{
	return strcmp(text, "1") == 0
		   || equalsWhileIgnoringCase(text, "true");
}


template <typename T>
inline T Tools::convert(const char* text)
{
	std::stringstream textStream(text);
	T result;
	textStream >> result;
	return result;
}


template<typename T>
inline std::string Tools::makeString(const T& content)
{
	std::stringstream result;
	result << content;
	return result.str();
}


template<typename EnumT>
inline constexpr int Tools::enumToInt(EnumT enumValue)
{
	return static_cast<std::underlying_type<EnumT>::type>(enumValue);
}


template <typename T1, typename T2>
std::string Tools::append(const T1& part1, const T2& part2)
{
	std::stringstream result;
	result << part1 << part2;
	return result.str();
}


template <typename T1, typename T2, typename T3>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3)
{
	std::stringstream result;
	result << part1 << part2 << part3;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4 << part5;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4 << part5 << part6;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4 << part5 << part6 << part7;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4 << part5 << part6 << part7 << part8;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4 << part5 << part6 << part7 << part8 << part9;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9, const T10& part10)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4 << part5 << part6 << part7 << part8 << part9 << part10;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9, const T10& part10, const T11& part11)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4 << part5 << part6 << part7 << part8 << part9 << part10 << part11;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9, const T10& part10, const T11& part11, const T12& part12)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4 << part5 << part6 << part7 << part8 << part9 << part10 << part11 << part12;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9, const T10& part10, const T11& part11, const T12& part12, const T13& part13)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4 << part5 << part6 << part7 << part8 << part9 << part10 << part11 << part12 << part13;
	return result.str();
}


template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
std::string Tools::append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9, const T10& part10, const T11& part11, const T12& part12, const T13& part13, const T14& part14)
{
	std::stringstream result;
	result << part1 << part2 << part3 << part4 << part5 << part6 << part7 << part8 << part9 << part10 << part11 << part12 << part13 << part14;
	return result.str();
}
