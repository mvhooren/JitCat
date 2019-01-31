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
	return static_cast<typename std::underlying_type<EnumT>::type>(enumValue);
}


template<typename... ValueTypes>
inline std::string Tools::append(ValueTypes&&... values)
{
	std::stringstream result;
	(result << ... << std::forward<ValueTypes>(values));
	return result.str();
}