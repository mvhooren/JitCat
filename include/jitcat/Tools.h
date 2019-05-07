/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace jitcat::Tools
{
	template<typename T>
	void deleteElements(std::vector<T>& vector);

	template <typename ContainerType>
	void deleteSecondElementsAndClear(ContainerType& map);

	template <typename T1, typename T2>
	bool isInList(const std::vector<T1>& vector, const T2& element);

	template <typename T>
	T convert(const std::string& text);

	template <typename T>
	T convert(const char* text);

	template <typename T>
	std::string makeString(const T& content);

	void split(const std::string& stringToSplit, const std::string& delims, std::vector<std::string>& stringsOut, bool allowEmpty = false);

	bool isNumber(const std::string& text);

	char toUpperCase(char text);
	std::string toUpperCase(std::string text);
	std::string toUpperCase(const char* text);
	std::string toUpperCase(const std::string_view& text);
	char toLowerCase(char text);
	std::string toLowerCase(std::string text);
	std::string toLowerCase(const char* text);
	std::string toLowerCase(const std::string_view& text);

	bool equalsWhileIgnoringCase(const std::string& text1, const std::string& text2);
	bool equalsWhileIgnoringCase(const std::string& text1, const char* text2);
	bool equalsWhileIgnoringCase(const char* text1, const std::string& text2);
	bool equalsWhileIgnoringCase(const char* text1, const char* text2);
	bool lessWhileIgnoringCase(const std::string& first, const std::string& second);

	std::string toHexBytes(const unsigned char* data, int length);

	template <typename EnumT>
	constexpr int enumToInt(EnumT enumValue);

	template <typename... ValueTypes>
	std::string append(ValueTypes&&... values);

	const std::string empty = "";

} //End namespace jitcat::Tools
#include "jitcat/ToolsHeaderImplementation.h"
