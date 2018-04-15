/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "Tools.h"

#include <cctype>


void Tools::split(const std::string& stringToSplit, const std::string& delims, std::vector<std::string>& stringsOut, bool allowEmpty)
{
	stringsOut.clear();
	std::string currentString;
	size_t a = 0;
	for (; a < stringToSplit.size(); a++)
	{
		size_t b = 0;
		for (; b < delims.size(); b++)
		{
			if (stringToSplit[a] == delims[b])
			{
				if (!currentString.empty()
					|| allowEmpty)
				{
					stringsOut.push_back(currentString);
					currentString.clear();
				}
				break;
			}
		}

		if (b == delims.size())
		{
			currentString.push_back(stringToSplit[a]);
		}
	}

	if (!currentString.empty())
	{
		stringsOut.push_back(currentString);
	}
}


bool Tools::isNumber(const std::string& text)
{
	// Go over all characters and check if all are either a digit, sign or dot. Allowing only the first character to be a sign and a single dot in the text.
	std::size_t textSize = text.size();
	if (textSize > 0)
	{
		bool pointFound = false;
		// Test first character separately.
		if (!std::isdigit(text[0])
			&& text[0] != '-'
			&& text[0] != '+')
		{
			if (text[0] != '.')
			{
				return false;
			}
			else
			{
				pointFound = true;
			}
		}
		for (std::size_t i = 1; i < textSize; i++)
		{
			if (!std::isdigit(text[i]))
			{
				if (text[i] != '.'
					|| pointFound)
				{
					return false;
				}
				else
				{
					pointFound = true;
				}
			}
		}
		return true;
	}
	// Empty string is not a number.
	return false;
}


char Tools::toUpperCase(char text)
{
	if (text <= 'z' && text >= 'a')
	{
		text = text + 'A' - 'a';
	}

	return text;
}


std::string Tools::toUpperCase(std::string text)
{
	int difference = 'A' - 'a';
	std::size_t size = text.size();
	for (std::size_t i = 0; i < size; ++i)
	{
		if (text[i] <= 'z' && text[i] >= 'a')
		{
			text[i] += char(difference);
		}
	}
	return text;
}


char Tools::toLowerCase(char text)
{
	if (text <= 'Z' && text >= 'A')
	{
		text = text + 'a' - 'A';
	}

	return text;
}


std::string Tools::toLowerCase(std::string text)
{
	const int difference = 'a' - 'A';
	std::size_t size = text.size();
	for (std::size_t i = 0; i < size; ++i)
	{
		if (text[i] <= 'Z' && text[i] >= 'A')
		{
			text[i] += char(difference);
		}
	}

	return text;
}


bool Tools::equalsWhileIgnoringCase(const std::string& text1, const std::string& text2)
{
	std::size_t text1Size = text1.size();
	std::size_t text2Size = text2.size();

	//early out if the strings are not of the same length
	if (text1Size != text2Size)
	{
		return false;
	}


	return equalsWhileIgnoringCase(text1.c_str(), text2.c_str());
}


bool Tools::equalsWhileIgnoringCase(const std::string& text1, const char* text2)
{
	return equalsWhileIgnoringCase(text1.c_str(), text2);
}


bool Tools::equalsWhileIgnoringCase(const char* text1, const std::string& text2)
{
	return equalsWhileIgnoringCase(text1, text2.c_str());
}


bool Tools::equalsWhileIgnoringCase(const char* text1, const char* text2)
{
	while (true)
	{
		//get the two characters we are comparing here, converted to upper case
		char char1 = toUpperCase(*text1);
		char char2 = toUpperCase(*text2);

		//Check whether there is a difference. This will also stop
		//when one of the two strings is shorter than the other
		if (char1 != char2)
		{
			return false;
		}

		//If one of them is at the end and we get here, then both are, which
		//means we reached the end without finding any differences.
		if (char1 == '\0')
		{
			return true;
		}

		++text1;
		++text2;
	}
}


const std::string Tools::empty = "";