/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ConstantToken.h"
#include "jitcat/Document.h"
#include "jitcat/Lexeme.h"
#include "jitcat/ParseHelper.h"

using namespace jitcat::Tokenizer;


ConstantToken::ConstantToken():
	subType(ConstantType::NoType)
{
}

ConstantToken::ConstantToken(const Lexeme& lexeme, ConstantType subType):
	ParseToken(lexeme),
	subType(subType)
{
}


int ConstantToken::getTokenID() const
{
	return getID();
}


const char* ConstantToken::getTokenName() const
{
	return "Constant";
}


ParseToken* ConstantToken::createIfMatch(Document* document, const char* currentPosition) const
{
	std::size_t  readLength = 0;
	ConstantType constant = parseConstant(currentPosition, document->getDocumentSize() - (currentPosition - document->getDocumentData().c_str()), readLength);
	if (constant != ConstantType::NoType)
	{
		Lexeme newLexeme = document->createLexeme(currentPosition - document->getDocumentData().c_str(), readLength);
		return new ConstantToken(newLexeme, constant);
	}
	else
	{
		return nullptr;
	}
}


const char* ConstantToken::getSubTypeName(int subType_) const
{
	switch ((ConstantType) subType_)
	{
		default:
		case ConstantType::Integer:			return "int";
		case ConstantType::FloatingPoint:	return "float";
		case ConstantType::String:			return "string";
		case ConstantType::Char:			return "char";
		case ConstantType::Bool:			return "bool";
	}
}


const char* ConstantToken::getSubTypeSymbol(int subType_) const
{
	return getSubTypeName(subType_);
}


int ConstantToken::getTokenSubType() const
{
	return (int)subType;
}


ConstantType ConstantToken::parseConstant(const char* text, std::size_t textLength, std::size_t & offset) const
{
	if (offset >= textLength)								return ConstantType::NoType;
	else if (ParseHelper::isNonZeroNumber(text[offset]))	return parseIntOrFloat(text, textLength, ++offset);
	else if (text[offset] == '0')							return parseFloatOrHexOrOct(text, textLength, ++offset);
	else if (text[offset] == '"')							return parseString(text, textLength, ++offset, false);
	else if (text[offset] == '\'')							return parseChar(text, textLength, ++offset);
	else if (ParseHelper::isAlphaNumeric(text[offset]))		return parseBool(text, textLength, offset);
	else													return ConstantType::NoType;
}


ConstantType ConstantToken::parseIntOrFloat(const char* text, std::size_t textLength, std::size_t & offset) const
{
	if (offset >= textLength)						return ConstantType::Integer;
	else if (ParseHelper::isNumber(text[offset]))	return parseIntOrFloat(text, textLength, ++offset);
	else if (text[offset] == '.')					return parseFloat(text, textLength, ++offset, true, false);
	else											return ConstantType::Integer;
}


ConstantType ConstantToken::parseFloatOrHexOrOct(const char* text, std::size_t textLength, std::size_t & offset) const
{
	if (offset >= textLength)							return ConstantType::Integer;
	else if (ParseHelper::isOctNumber(text[offset]))	return parseFloatOrOct(text, textLength, ++offset);
	else if (ParseHelper::isNumber(text[offset]))		return parseFloat(text, textLength, ++offset, false, false);
	else if (text[offset] == '.')						return parseFloat(text, textLength, ++offset, true, false);
	else if (text[offset] == 'x'
			 || text[offset] == 'X')					return parseHex(text, textLength, ++offset);
	else												return ConstantType::Integer;

}


ConstantType ConstantToken::parseFloatOrOct(const char* text, std::size_t textLength, std::size_t & offset) const
{
	if (offset >= textLength)
	{
		return ConstantType::Integer;
	}
	else if (ParseHelper::isOctNumber(text[offset]))
	{
		return parseFloatOrOct(text, textLength, ++offset);
	}
	else if (ParseHelper::isNumber(text[offset]))
	{
		return parseFloat(text, textLength, ++offset, false, false);
	}
	else if (text[offset] == '.')
	{
		return parseFloat(text, textLength, ++offset, true, false);
	}
	else
	{
		return ConstantType::Integer;
	}
}


ConstantType ConstantToken::parseFloat(const char* text, std::size_t textLength, std::size_t & offset, bool pastDot, bool pastExponent) const
{
	if (offset >= textLength)
	{
		if (pastDot && ParseHelper::isNumber(text[offset - 1]))
		{
			return ConstantType::FloatingPoint;
		}
		else
		{
			return ConstantType::NoType;
		}
	}
	else if (!pastExponent
			 && ParseHelper::isNumber(text[offset]))
	{
		return parseFloat(text, textLength, ++offset, pastDot, pastExponent);
	}
	else if (!pastDot
			 && text[offset] == '.')
	{
		return parseFloat(text, textLength, ++offset, true, pastExponent);
	}
	else if (pastDot && !pastExponent 
		     && (text[offset] == 'e' || text[offset] == 'E'))
	{
		std::size_t  exponentOffset = 0;
		if (parseFloatExponent(text + offset + 1, textLength, exponentOffset))
		{
			offset += exponentOffset + 1;
			return parseFloat(text, textLength, offset, pastDot, true);
		}
		else
		{
			if (pastDot && ParseHelper::isNumber(text[offset - 1]))
			{
				return ConstantType::FloatingPoint;
			}
			else
			{
				return ConstantType::NoType;
			}
		}
	}
	else if (pastDot && ParseHelper::isNumber(text[offset - 1]) 
		     && (text[offset] == 'f' || text[offset] == 'F'))
	{
		offset++;
		return ConstantType::FloatingPoint;
	}
	else if (pastDot && ParseHelper::isNumber(text[offset - 1]))
	{
		return ConstantType::FloatingPoint;
	}
	else
	{
		return ConstantType::NoType;
	}
}


bool ConstantToken::parseFloatExponent(const char* text, std::size_t textLength, std::size_t & offset) const
{
	if (offset >= textLength)
	{
		return offset >= 1 && !(offset == 1 && text[0] == '-');
	}
	else if (offset == 0
			 && text[offset] == '-')
	{
		return parseFloatExponent(text, textLength, ++offset);
	}
	else if (ParseHelper::isNumber(text[offset]))
	{
		return parseFloatExponent(text, textLength, ++offset);
	}
	else if (offset >= 1 && !(offset == 1 && text[0] == '-'))
	{
		return true;
	}
	return false;
}


ConstantType ConstantToken::parseHex(const char* text, std::size_t textLength, std::size_t & offset) const
{
	if (offset >= textLength
		|| !ParseHelper::isHexDigit(text[offset]))
	{
		if (offset > 1)
		{
			return ConstantType::Integer;
		}
		else
		{
			return ConstantType::NoType;
		}
	}
	else if (ParseHelper::isHexDigit(text[offset]))
	{
		return parseHex(text, textLength, ++offset);
	}
	else
	{
		return ConstantType::NoType;
	}
}


ConstantType ConstantToken::parseString(const char* text, std::size_t textLength, std::size_t & offset, bool escaped) const
{
	if (offset >= textLength)
	{
		return ConstantType::NoType;
	}
	else if (text[offset] == '"' && !escaped)
	{
		offset++;
		return ConstantType::String;
	}
	else if (text[offset] == '\\' && !escaped)
	{
		return parseString(text, textLength, ++offset, true);
	}
	else if (ParseHelper::isValidStringChar(text[offset]))
	{
		return parseString(text, textLength, ++offset, false);
	}
	else 
	{
		return ConstantType::NoType;
	}
}


ConstantType ConstantToken::parseChar(const char* text, std::size_t textLength, std::size_t & offset) const
{
	if (textLength - offset >= 2)
	{
		if (ParseHelper::isValidStringChar(text[offset]) && text[offset + 1] == '\'')
		{
			offset += 2;
			return ConstantType::Char;
		}
		else if (textLength - offset >= 3
			     && text[offset] == '\\'
			     && ParseHelper::isValidStringChar(text[offset + 1]) 
				 && text[offset + 2] == '\'')
		{
			offset += 3;
			return ConstantType::Char;
		}
	}
	return ConstantType::NoType;
}


ConstantType ConstantToken::parseBool(const char* text, std::size_t textLength, std::size_t & offset) const
{
	if (textLength - offset >= 4)
	{
		if ((text[0] == 't' || text[0] == 'T') && (text[1] == 'r' || text[1] == 'R') && (text[2] == 'u' || text[2] == 'U') && (text[3] == 'e' || text[3] == 'E'))
		{
			offset += 4;
			return ConstantType::Bool;
		}
		else if((text[0] == 'f' || text[0] == 'F') && (text[1] == 'a' || text[1] == 'A') && (text[2] == 'l' || text[2] == 'L') && (text[3] == 's' || text[3] == 'S') && (text[4] == 'e' || text[4] == 'E'))
		{
			offset += 5;
			return ConstantType::Bool;
		}
		else
		{
			return ConstantType::NoType;
		}
	}
	else
	{
		return ConstantType::NoType;
	}
}

const int ConstantToken::getID()
{
	static int ID = ParseToken::getNextTokenID(); 
	return ID;
}