/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Tools.h"

#include <cassert>
#include <cctype>
#include <cwctype>

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


bool jitcat::Tools::startsWith(const std::string& text, const std::string& prefix)
{
	return !(prefix.size() > text.size())
		   && text.substr(0, prefix.size()) == prefix;
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


uintptr_t jitcat::Tools::alignPointer(uintptr_t pointer, std::size_t alignment)
{
	return (pointer + alignment - 1) & ~((uintptr_t)alignment - 1);
}


std::size_t jitcat::Tools::roundUp(std::size_t size, std::size_t multiple)
{
	if (multiple == 0)
		return size;

	std::size_t remainder = size % multiple;
	if (remainder == 0)
		return size;

	return size + multiple - remainder;
}


std::string jitcat::Tools::replaceInString(const std::string& original, const std::string& toReplace, const std::string& replacement)
{
	std::string workingString = original;
	typename std::string::size_type lastPosition = std::string::npos;
	do
	{
		typename std::string::size_type position = workingString.find(toReplace);
		if (position == std::string::npos || position == lastPosition)
		{
			return workingString;
		}
		else
		{
			workingString.replace(position, toReplace.size(), replacement);
		}
		lastPosition = position;
	} while (true);

}


std::string jitcat::Tools::toXMLCompatible(const std::string& text)
{
	std::string escaped = text;
	for (std::size_t i = 0; i < escaped.size(); ++i)
	{
		switch (escaped[i])
		{
			case '>':	escaped.replace(i, 1, "&gt;"); i += 3; break;
			case '<':	escaped.replace(i, 1, "&lt;"); i += 3; break;
			case '&':	escaped.replace(i, 1, "&amp;"); i += 4; break;
			case '"':	escaped.replace(i, 1, "&quot;"); i += 5; break;
			case '\'':	escaped.replace(i, 1, "&apos;"); i += 5; break;
			default: continue;
		}
	}
	return escaped;
}


std::string jitcat::Tools::fromXMLCompatible(const std::string& text)
{
	std::ostringstream unescaped;
	for (std::size_t i = 0; i < text.size(); ++i)
	{
		if (text[i] == '&')
		{
			int textSizeRemaining = (int)text.size() - (int)i;
			if (textSizeRemaining >= 4)
			{
				if (text[i + 2] == 't' && text[i + 3] == ';')
				{
					if (text[i + 1] == 'g')
					{
						unescaped << ">"; i += 3; continue;
					}
					else if (text[i + 1] == 'l')
					{
						unescaped << "<"; i += 3; continue;
					}
				}
				else if (textSizeRemaining >= 5 && text[i + 4] == ';' && text[i + 1] == 'a' && text[i + 2] == 'm' && text[i + 3] == 'p')
				{
					unescaped << "&"; i += 4; continue;
				}
				else if (textSizeRemaining >= 6 && text[i + 5] == ';' && text[i + 3] == 'o')
				{
					if (text[i + 1] == 'q' && text[i + 2] == 'u' && text[i + 4] == 't')
					{
						unescaped << "\""; i += 5; continue;
					}
					else if (text[i + 1] == 'a' && text[i + 2] == 'p' && text[i + 4] == 's')
					{
						unescaped << "\""; i += 5; continue;
					}
				}
			}
		}
		unescaped << text[i];
	}
	return unescaped.str();
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


wchar_t jitcat::Tools::toUpperCase(wchar_t text)
{
	return std::towupper(text);
}


std::wstring jitcat::Tools::toUpperCase(std::wstring text)
{
	std::size_t size = text.size();
	for (std::size_t i = 0; i < size; ++i)
	{
		text[i] = toUpperCase(text[i]);
	}
	return text;
}


std::wstring jitcat::Tools::toUpperCase(const wchar_t* text)
{
	return toUpperCase(std::wstring(text));
}


std::wstring jitcat::Tools::toUpperCase(const std::wstring_view& text)
{
	return toUpperCase(std::wstring(text));
}


wchar_t jitcat::Tools::toLowerCase(wchar_t text)
{
	return std::towlower(text);
}


std::wstring jitcat::Tools::toLowerCase(std::wstring text)
{
	std::size_t size = text.size();
	for (std::size_t i = 0; i < size; ++i)
	{
		text[i] = toLowerCase(text[i]);
	}
	return text;
}


std::wstring jitcat::Tools::toLowerCase(const wchar_t* text)
{
	return toLowerCase(std::wstring(text));
}


std::wstring jitcat::Tools::toLowerCase(const std::wstring_view& text)
{
	return toLowerCase(std::wstring(text));
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


bool jitcat::Tools::equalsWhileIgnoringCase(const std::wstring& text1, const std::wstring& text2)
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


bool jitcat::Tools::equalsWhileIgnoringCase(const std::wstring& text1, const wchar_t* text2)
{
	return equalsWhileIgnoringCase(text1.c_str(), text2);
}


bool jitcat::Tools::equalsWhileIgnoringCase(const wchar_t* text1, const std::wstring& text2)
{
	return equalsWhileIgnoringCase(text1, text2.c_str());
}


bool jitcat::Tools::equalsWhileIgnoringCase(const wchar_t* text1, const wchar_t* text2)
{
	while (true)
	{
		//get the two characters we are comparing here, converted to upper case
		wchar_t char1 = toUpperCase(*text1);
		wchar_t char2 = toUpperCase(*text2);

		//Check whether there is a difference. This will also stop
		//when one of the two strings is shorter than the other
		if (char1 != char2)
		{
			return false;
		}

		//If one of them is at the end and we get here, then both are, which
		//means we reached the end without finding any differences.
		if (char1 == L'\0')
		{
			return true;
		}

		++text1;
		++text2;
	}
}


bool jitcat::Tools::lessWhileIgnoringCase(const std::wstring& text1, const std::wstring& text2)
{
	const wchar_t* charText1 = text1.c_str();
	const wchar_t* charText2 = text2.c_str();
	while (true)
	{
		//Check whether char1 is maller than char2. Also automatically checks if text1 is shorter than text2, because the last character is always 0.
		if (*charText1 != *charText2)
		{
			//get the two characters we are comparing here, converted to upper case
			wchar_t char1 = toUpperCase(*charText1);
			wchar_t char2 = toUpperCase(*charText2);

			//We want underscore ('_') to be before all other characters, but NOT before whitespaces.
			//Do this by simply changing their code to some weird ASCII codes that are never used anyway:
			//30 is RS (record separator)
			//31 is US (unit separator)
			if (char1 == L' ')		char1 = 30;
			else if (char1 == L'_')	char1 = 31;

			if (char2 == L' ')		char2 = 30;
			else if (char2 == L'_')	char2 = 31;

			if (char1 != char2)
			{
				return char1 < char2;
			}
		}

		//If char2 is 0, the strings are either equal or text1 is a longer string. In both cases, text1 is not smaller than text2.
		if (*charText2 == L'\0')
		{
			return false;
		}

		++charText1;
		++charText2;
	}
}


std::string jitcat::Tools::toHexBytes(std::size_t number, bool spaced)
{
	return toHexBytes(reinterpret_cast<const unsigned char*>(&number), sizeof(std::size_t), spaced);
}


std::string jitcat::Tools::toHexBytes(const unsigned char* data, int length, bool spaced)
{
	std::stringstream stream;
	for (int i = 0; i < length; i++)
	{
		int byteValue = (int)data[i];
		if (byteValue <= 0xf)
		{
			stream << "0";
		}
		stream << std::hex << byteValue;
		if (spaced)
			stream << " ";
	}
	return stream.str();
}


std::size_t jitcat::Tools::reverseBytes(std::size_t value)
{
	if constexpr (sizeof(std::size_t) == 8)
	{
		char buffer[8];
		memcpy(buffer, &value, 8);
		std::swap(buffer[7], buffer[0]);
		std::swap(buffer[6], buffer[1]);
		std::swap(buffer[5], buffer[2]);
		std::swap(buffer[4], buffer[3]);
		return *reinterpret_cast<std::size_t*>(buffer);
	}
	else
	{
		char buffer[4];
		memcpy(buffer, &value, 4);
		std::swap(buffer[3], buffer[0]);
		std::swap(buffer[2], buffer[1]);
		return *reinterpret_cast<std::size_t*>(buffer);
	}
}


std::size_t jitcat::Tools::hashCombine(std::size_t firstHash, std::size_t secondHash)
{
	if constexpr (sizeof(std::size_t) == 8)
	{
		const unsigned long long kMul = 0x9ddfea08eb382d69ULL;
		unsigned long long a = (firstHash ^ secondHash) * kMul;
		a ^= (a >> 47);
		unsigned long long b = (secondHash ^ a) * kMul;
		b ^= (b >> 47);
		secondHash = (std::size_t)(b * kMul);
		return secondHash;
	}
	else
	{
		return secondHash ^ ( firstHash + 0x9e3779b9 + (secondHash<<6) + (secondHash>>2));
	}
}
