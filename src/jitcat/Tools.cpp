/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Tools.h"

#include <cctype>

using namespace jitcat::Tools;


void jitcat::Tools::split(const std::string& stringToSplit, const std::string& delims, std::vector<std::string>& stringsOut, bool allowEmpty)
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


bool jitcat::Tools::isNumber(const std::string& text)
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


char jitcat::Tools::toUpperCase(char text)
{
	if (text <= 'z' && text >= 'a')
	{
		text = text + 'A' - 'a';
	}

	return text;
}


std::string jitcat::Tools::toUpperCase(std::string text)
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


std::string jitcat::Tools::toUpperCase(const char* text)
{
	return toUpperCase(std::string(text));
}


std::string jitcat::Tools::toUpperCase(const std::string_view& text)
{
	std::string textCopy(text);
	return toUpperCase(textCopy);
}


char jitcat::Tools::toLowerCase(char text)
{
	if (text <= 'Z' && text >= 'A')
	{
		text = text + 'a' - 'A';
	}

	return text;
}


std::string jitcat::Tools::toLowerCase(std::string text)
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


std::string jitcat::Tools::toLowerCase(const char* text)
{
	return toLowerCase(std::string(text));
}


std::string jitcat::Tools::toLowerCase(const std::string_view& text)
{
	std::string textCopy(text);
	return toLowerCase(textCopy);
}


bool jitcat::Tools::equalsWhileIgnoringCase(const std::string& text1, const std::string& text2)
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


bool jitcat::Tools::equalsWhileIgnoringCase(const std::string& text1, const char* text2)
{
	return equalsWhileIgnoringCase(text1.c_str(), text2);
}


bool jitcat::Tools::equalsWhileIgnoringCase(const char* text1, const std::string& text2)
{
	return equalsWhileIgnoringCase(text1, text2.c_str());
}


bool jitcat::Tools::equalsWhileIgnoringCase(const char* text1, const char* text2)
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


bool jitcat::Tools::lessWhileIgnoringCase(const std::string& text1, const std::string& text2)
{
	const char* charText1 = text1.c_str();
	const char* charText2 = text2.c_str();
	while (true)
	{
		//Check whether char1 is maller than char2. Also automatically checks if text1 is shorter than text2, because the last character is always 0.
		if (*charText1 != *charText2)
		{
			//get the two characters we are comparing here, converted to upper case
			char char1 = toUpperCase(*charText1);
			char char2 = toUpperCase(*charText2);

			//We want underscore ('_') to be before all other characters, but NOT before whitespaces.
			//Do this by simply changing their code to some weird ASCII codes that are never used anyway:
			//30 is RS (record separator)
			//31 is US (unit separator)
			if (char1 == ' ')		char1 = 30;
			else if (char1 == '_')	char1 = 31;

			if (char2 == ' ')		char2 = 30;
			else if (char2 == '_')	char2 = 31;

			if (char1 != char2)
			{
				return char1 < char2;
			}
		}

		//If char2 is 0, the strings are either equal or text1 is a longer string. In both cases, text1 is not smaller than text2.
		if (*charText2 == '\0')
		{
			return false;
		}

		++charText1;
		++charText2;
	}
}


std::string jitcat::Tools::toHexBytes(const unsigned char* data, int length)
{
	std::stringstream stream;
	for (int i = 0; i < length; i++)
	{
		int byteValue = (int)data[i];
		if (byteValue <= 0xf)
		{
			stream << "0";
		}
		stream << std::hex << byteValue  << " ";
	}
	return stream.str();
}