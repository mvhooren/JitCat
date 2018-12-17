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

	template <typename T1, typename T2>
	static std::string append(const T1& part1, const T2& part2);

	template <typename T1, typename T2, typename T3>
	static std::string append(const T1& part1, const T2& part2, const T3& part3);

	template <typename T1, typename T2, typename T3, typename T4>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4);

	template <typename T1, typename T2, typename T3, typename T4, typename T5>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5);

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6);

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7);

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8);

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9);

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9, const T10& part10);

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9, const T10& part10, const T11& part11);

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9, const T10& part10, const T11& part11, const T12& part12);

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9, const T10& part10, const T11& part11, const T12& part12, const T13& part13);

	template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12, typename T13, typename T14>
	static std::string append(const T1& part1, const T2& part2, const T3& part3, const T4& part4, const T5& part5, const T6& part6, const T7& part7, const T8& part8, const T9& part9, const T10& part10, const T11& part11, const T12& part12, const T13& part13, const T14& part14);

	static const std::string empty;
};


#include "ToolsHeaderImplementation.h"
