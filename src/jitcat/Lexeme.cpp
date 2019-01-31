/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "Lexeme.h"
#include "Document.h"

Lexeme::Lexeme(Document* document, std::size_t offset, std::size_t length):
	document(document),
	offset(offset),
	length(length)
{
}

const char* Lexeme::getDataPointer() const
{
	if (document->getDocumentSize() >= offset + length)
	{
		return document->getDocumentData() + offset;
	}
	else
	{
		return 0;
	}
}


std::string Lexeme::toString() const
{
	return std::string(getDataPointer(), length);
}


bool operator==(const Lexeme& lexeme, const char* other)
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


bool operator!=(const Lexeme & lexeme, const char * other)
{
	return !(lexeme == other);
}

bool operator==(const Lexeme& lexeme, const std::string& other)
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


bool operator!=(const Lexeme& lexeme, const std::string& other)
{
	return !(lexeme == other);
}


std::ostream& operator<<(std::ostream& stream, const Lexeme* lexeme)
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



