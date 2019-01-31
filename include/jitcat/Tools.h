/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <string>
#include <vector>

class Tools
{
public:
	template<typename T>
	static void deleteElements(std::vector<T>& vector);

	template <typename ContainerType>
	static void deleteSecondElementsAndClear(ContainerType& map);

	template <typename T1, typename T2>
	static bool isInList(const std::vector<T1>& vector, const T2& element);

	template <typename T>
	static T convert(const std::string& text);

	template <typename T>
	static T convert(const char* text);

	template <typename T>
	static std::string makeString(const T& content);

	static void split(const std::string& stringToSplit, const std::string& delims, std::vector<std::string>& stringsOut, bool allowEmpty = false);

	static bool isNumber(const std::string& text);

	static char toUpperCase(char text);
	static std::string toUpperCase(std::string text);
	static char toLowerCase(char text);
	static std::string toLowerCase(std::string text);

	static bool equalsWhileIgnoringCase(const std::string& text1, const std::string& text2);
	static bool equalsWhileIgnoringCase(const std::string& text1, const char* text2);
	static bool equalsWhileIgnoringCase(const char* text1, const std::string& text2);
	static bool equalsWhileIgnoringCase(const char* text1, const char* text2);

	static std::string toHexBytes(const unsigned char* data, int length);

	template <typename EnumT>
	static constexpr int enumToInt(EnumT enumValue);

	template <typename... ValueTypes>
	static std::string append(ValueTypes&&... values);

	static const std::string empty;
};


#include "ToolsHeaderImplementation.h"
