/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class Document;
#include <iostream>
#include <string.h>

//A lexeme points to a word or symbol contained in a document.
//It is typically used to refer to the document text represented by a ParseToken.
struct Lexeme
{
	Lexeme(Document* document, std::size_t offset, std::size_t length);
	//Get a pointer to the contents of the document referred to by this Lexeme.
	//The const char* is valid as long as the document is not edited.
	const char* getDataPointer() const;
	Document* document;
	std::size_t offset;
	std::size_t length;

	std::string toString() const
	{
		return std::string(getDataPointer(), length);
	}

	friend bool operator==(const Lexeme& lexeme, const char* other)
	{
		const char* data = lexeme.getDataPointer();
		std::size_t otherLength = strlen(other);
		if (lexeme.length == 0 && otherLength == 0)
		{
			return true;
		}
		if (lexeme.length == otherLength
			&& data != nullptr)
		{
			for (unsigned int i = 0; i < lexeme.length; i++)
			{
				if (data[i] != other[i])
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}


	friend bool operator!=(const Lexeme& lexeme, const char* other)
	{
		return !(lexeme == other);
	}


	friend bool operator==(const Lexeme& lexeme, const std::string& other)
	{
		const char* data = lexeme.getDataPointer();
		std::size_t otherLength = other.length();
		if (lexeme.length == 0 && otherLength == 0)
		{
			return true;
		}
		if (lexeme.length == other.length()
			&& data != nullptr)
		{
			for (unsigned int i = 0; i < lexeme.length; i++)
			{
				if (data[i] != other[i])
				{
					return false;
				}
			}
			return true;
		}
		return false;
	}


	friend bool operator!=(const Lexeme& lexeme, const std::string& other)
	{
		return !(lexeme == other);
	}


	friend std::ostream& operator<< (std::ostream& stream, const Lexeme* lexeme)
	{
		const char* data = lexeme->getDataPointer();
		if (data != nullptr)
		{
			for (unsigned int i = 0; i < lexeme->length; i++)
			{
				if (data[i] == '\n')
				{
					stream << "\\n";
				}
				else if (data[i] == '\t')
				{
					stream << "\\t";
				}
				else
				{
					stream << data[i];
				}
			}

		}
		return stream;
	}
};