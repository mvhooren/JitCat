/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Tools.h"

#include <sstream>
#include <string.h>

namespace jitcat::Tools
{

	template<typename T>
	inline void deleteElements(std::vector<T>& vector)
	{
		for (auto& i : vector)
		{
			delete i;
		}
		vector.clear();
	}


	template<typename ContainerType>
	inline void deleteSecondElementsAndClear(ContainerType& map)
	{
		typename ContainerType::const_iterator end = map.end();
		for (typename ContainerType::iterator iter = map.begin(); iter != end; ++iter)
		{
			delete (*iter).second;
		}
		map.clear();
	}


	template<typename T1, typename T2>
	inline bool isInList(const std::vector<T1>& vector, const T2 & element)
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
	inline T convert(const std::string& text)
	{
		return convert<T>(text.c_str());
	}


	template<>
	inline bool convert<bool>(const char* text)
	{
		return strcmp(text, "1") == 0
			   || equalsWhileIgnoringCase(text, "true");
	}


	template <typename T>
	inline T convert(const char* text)
	{
		std::stringstream textStream(text);
		T result;
		textStream >> result;
		return result;
	}


	template<typename T>
	inline std::string makeString(const T& content)
	{
		std::stringstream result;
		result << content;
		return result.str();
	}


	template<typename EnumT>
	inline constexpr int enumToInt(EnumT enumValue)
	{
		return static_cast<typename std::underlying_type<EnumT>::type>(enumValue);
	}


	template<typename... ValueTypes>
	inline std::string append(ValueTypes&&... values)
	{
		std::stringstream result;
		(result << ... << std::forward<ValueTypes>(values));
		return result.str();
	}

} //End namespace jitcat::Tools