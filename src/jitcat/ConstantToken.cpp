/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ConstantToken.h"
#include "jitcat/Document.h"
#include "jitcat/Configuration.h"
#include "jitcat/Lexeme.h"
#include "jitcat/ParseHelper.h"
#include "jitcat/Tools.h"

using namespace jitcat::Tokenizer;


const char* ConstantToken::getTokenName() const
{
	return "Constant";
}


bool ConstantToken::createIfMatch(Document& document, std::size_t& currentPosition) const
{
	std::size_t  readLength = 0;
	const char* currentCharacter = &document.getDocumentData()[currentPosition];
	ConstantType constant = parseConstant(currentCharacter, document.getDocumentSize() - currentPosition, readLength);
	if (constant != ConstantType::NoType)
	{
		document.addToken(currentPosition, readLength, id, Tools::enumToUSHort(constant));
		currentPosition += readLength;
		return true;
	}
	else
	{
		return false;
	}
}


const char* ConstantToken::getSubTypeName(unsigned short subType_) const
{
	switch ((ConstantType) subType_)
	{
		default:
		case ConstantType::Integer:				return "int_literal";
		case ConstantType::DoubleFloatingPoint:	return "double_literal";
		case ConstantType::FloatingPoint:		return "float_literal";
		case ConstantType::String:				return "string_literal";
		case ConstantType::Char:				return "char_literal";
		case ConstantType::Bool:				return "bool_literal";
	}
}


const char* ConstantToken::getSubTypeSymbol(unsigned short subType_) const
{
	return getSubTypeName(subType_);
}


ConstantType ConstantToken::parseConstant(const char* text, std::size_t textLength, std::size_t & offset) const
{
	if (offset >= textLength)								return ConstantType::NoType;
	else if (ParseHelper::isNonZeroNumber(text[offset]))	return parseIntOrFloat(text, textLength, ++offset);
	else if (text[offset] == '.')							return parseFloat(text, textLength, ++offset, true, false);
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
	else if ((text[offset] == 'e')
			 || (text[offset] == 'E'))				return parseFloatWithExponent(text, textLength, ++offset, false);
	else											return ConstantType::Integer;
}


ConstantType ConstantToken::parseFloatOrHexOrOct(const char* text, std::size_t textLength, std::size_t & offset) const
{
	if (offset >= textLength)							return ConstantType::Integer;
	else if (ParseHelper::isOctNumber(text[offset]))	return parseFloatOrOct(text, textLength, ++offset);
	else if (ParseHelper::isNumber(text[offset]))		return parseFloat(text, textLength, ++offset, false, false);
	else if (text[offset] == '.')						return parseFloat(text, textLength, ++offset, true, false);
	else if ((text[offset] == 'e')
			 || (text[offset] == 'E'))					return parseFloatWithExponent(text, textLength, ++offset, false);
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
		if ((pastDot || pastExponent) && offset > 1)
		{
			if constexpr (Configuration::defaultFloatingPointLiteralIsDouble)
			{
				return ConstantType::DoubleFloatingPoint;
			}
			else
			{
				return ConstantType::FloatingPoint;
			}

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
	else if (!pastDot && !pastExponent
			 && text[offset] == '.')
	{
		return parseFloat(text, textLength, ++offset, true, pastExponent);
	}
	else if (pastDot && !pastExponent 
		     && (text[offset] == 'e' || text[offset] == 'E')
		     && offset > 1)
	{
		return parseFloatWithExponent(text, textLength, ++offset, pastDot);
	}
	else if (((pastDot && offset > 1) || pastExponent) 
		     && (text[offset] == 'f' || text[offset] == 'F'))
	{
		offset++;
		return ConstantType::FloatingPoint;
	}
	else if (((pastDot && offset > 1) || pastExponent) 
		     && (text[offset] == 'd' || text[offset] == 'D'))
	{
		offset++;
		return ConstantType::DoubleFloatingPoint;
	}
	else if (pastDot && offset > 1)
	{
		if constexpr (Configuration::defaultFloatingPointLiteralIsDouble)
		{
			return ConstantType::DoubleFloatingPoint;
		}
		else
		{
			return ConstantType::FloatingPoint;
		}
	}
	else
	{
		return ConstantType::NoType;
	}
}


ConstantType jitcat::Tokenizer::ConstantToken::parseFloatWithExponent(const char* text, std::size_t textLength, std::size_t& offset, bool pastDot) const
{
	std::size_t  exponentOffset = 0;
	if (parseFloatExponent(text + offset, textLength, exponentOffset))
	{
		offset += exponentOffset;
		return parseFloat(text, textLength, offset, pastDot, true);
	}
	else
	{
		if (pastDot && ParseHelper::isNumber(text[offset - 1]))
		{
			return ConstantType::DoubleFloatingPoint;
		}
		else
		{
			return ConstantType::NoType;
		}
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
		if ((text[0] == 't' || text[0] == 'T') && (text[1] == 'r' || text[1] == 'R') && (text[2] == 'u' || text[2] == 'U') && (text[3] == 'e' || text[3] == 'E') 
			&& !ParseHelper::isAlphaNumeric(text[4]) && text[4] != '_') 
		{
			offset += 4;
			return ConstantType::Bool;
		}
		else if((text[0] == 'f' || text[0] == 'F') && (text[1] == 'a' || text[1] == 'A') && (text[2] == 'l' || text[2] == 'L') && (text[3] == 's' || text[3] == 'S') && (text[4] == 'e' || text[4] == 'E')
				&& !ParseHelper::isAlphaNumeric(text[5]) && text[5] != '_')
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
