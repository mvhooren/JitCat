/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/OneCharToken.h"
#include "jitcat/Document.h"
#include "jitcat/Lexeme.h"
#include "jitcat/Tools.h"

using namespace jitcat::Tokenizer;


const char* OneCharToken::getTokenName() const
{
	return "OneChar";
}


const char* OneCharToken::getSubTypeName(unsigned short subType_) const
{
	switch ((OneChar) subType_)
	{
		default:
		case OneChar::Unknown:			return "unknown";
		case OneChar::BraceOpen:		return "brace open";
		case OneChar::BraceClose:		return "brace close";
		case OneChar::ParenthesesOpen:	return "parentheses open";
		case OneChar::ParenthesesClose:	return "parentheses close";
		case OneChar::BracketOpen:		return "bracket open";
		case OneChar::BracketClose:		return "bracket close";
		case OneChar::Assignment:		return "assignment";
		case OneChar::Plus:				return "plus";
		case OneChar::Times:			return "times";
		case OneChar::Divide:			return "divide";
		case OneChar::Minus:			return "minus";
		case OneChar::Modulo:			return "modulo";
		case OneChar::Smaller:			return "smaller";
		case OneChar::Greater:			return "greater";
		case OneChar::Comma:			return "comma";
		case OneChar::Semicolon:		return "semicolon";
		case OneChar::BitwiseAnd:		return "bitwise and";
		case OneChar::BitwiseOr:		return "bitwise or";
		case OneChar::BitwiseXor:		return "bitwise xor";
		case OneChar::Not:				return "bitwise not";
		case OneChar::Dot:				return "dot";
		case OneChar::At:				return "at";
	}
}


const char* OneCharToken::getSubTypeSymbol(unsigned short subType_) const
{
	switch ((OneChar) subType_)
	{
		default:
		case OneChar::Unknown:			return "?";
		case OneChar::BraceOpen:		return "{";
		case OneChar::BraceClose:		return "}";
		case OneChar::ParenthesesOpen:	return "(";
		case OneChar::ParenthesesClose:	return ")";
		case OneChar::BracketOpen:		return "[";
		case OneChar::BracketClose:		return "]";
		case OneChar::Assignment:		return "=";
		case OneChar::Plus:				return "+";
		case OneChar::Times:			return "*";
		case OneChar::Divide:			return "/";
		case OneChar::Minus:			return "-";
		case OneChar::Modulo:			return "%";
		case OneChar::Smaller:			return "<";
		case OneChar::Greater:			return ">";
		case OneChar::Comma:			return ",";
		case OneChar::Semicolon:		return ";";
		case OneChar::BitwiseAnd:		return "&";
		case OneChar::BitwiseOr:		return "|";
		case OneChar::BitwiseXor:		return "^";
		case OneChar::Not:				return "!";
		case OneChar::Dot:				return ".";
		case OneChar::At:				return "@";
	}
}


bool OneCharToken::isSuggestedToken(unsigned short subType_) const
{
	OneChar tokenType = static_cast<OneChar>(subType_);
	return	   tokenType == OneChar::BraceClose 
			|| tokenType == OneChar::ParenthesesClose 
			|| tokenType == OneChar::BracketClose 
			|| tokenType == OneChar::Semicolon;
}


bool OneCharToken::createIfMatch(Document& document, std::size_t& currentPosition) const
{
	std::size_t remainingLength = document.getDocumentSize() - currentPosition;
	const char* currentCharacter = &document.getDocumentData()[currentPosition];
	if (remainingLength > 0)
	{
		OneChar type = OneChar::Unknown;
		switch (currentCharacter[0])
		{
			case '{':	type = OneChar::BraceOpen;			break;
			case '}':	type = OneChar::BraceClose;			break;
			case '(':	type = OneChar::ParenthesesOpen;	break;
			case ')':	type = OneChar::ParenthesesClose;	break;
			case '[':	type = OneChar::BracketOpen;		break;
			case ']':	type = OneChar::BracketClose;		break;
			case '=':	type = OneChar::Assignment;			break;
			case '+':	type = OneChar::Plus;				break;
			case '*':	type = OneChar::Times;				break;
			case '/':	type = OneChar::Divide;				break;
			case '-':	type = OneChar::Minus;				break;
			case '%':	type = OneChar::Modulo;				break;
			case '<':	type = OneChar::Smaller;			break;
			case '>':	type = OneChar::Greater;			break;
			case ',':	type = OneChar::Comma;				break;
			case ';':	type = OneChar::Semicolon;			break;
			case '&':	type = OneChar::BitwiseAnd;			break;
			case '|':	type = OneChar::BitwiseOr;			break;
			case '^':	type = OneChar::BitwiseXor;			break;
			case '!':	type = OneChar::Not;				break;
			case '.':	type = OneChar::Dot;				break;
			case '@':	type = OneChar::At;					break;
		}
		if (type != OneChar::Unknown)
		{
			document.addToken(currentPosition, 1, id, Tools::enumToUSHort(type));
			currentPosition++;
			return true;
		}
	}
	return false;
}