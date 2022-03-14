/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/TwoCharToken.h"
#include "jitcat/Document.h"
#include "jitcat/Lexeme.h"
#include "jitcat/Tools.h"

using namespace jitcat::Tokenizer;


const char* TwoCharToken::getTokenName() const
{
	return "TwoChar";
}


const char* TwoCharToken::getSubTypeName(unsigned short subType_) const
{
	switch ((TwoChar) subType_)
	{
		default:
		case TwoChar::Unknown:			return "unknown";
		case TwoChar::Equals:			return "equals";
		case TwoChar::NotEquals:		return "not equals";
		case TwoChar::SmallerOrEqual:	return "smaller or equal";
		case TwoChar::GreaterOrEqual:	return "greater or equal";
		case TwoChar::LogicalAnd:		return "logical and";
		case TwoChar::LogicalOr:		return "logical or";
		case TwoChar::PlusAssign:		return "plus assign";
		case TwoChar::MinusAssign:		return "minus assign";
		case TwoChar::TimesAssign:		return "times assign";
		case TwoChar::DivideAssign:		return "divide assign";
		case TwoChar::BitwiseOrAssign:	return "bitwise or assign";
		case TwoChar::BitwiseAndAssign:	return "bitwise and assign";
		case TwoChar::BitwiseXorAssign:	return "bitwise xor assign";
		case TwoChar::BitshiftLeft:		return "bitshift left";
		case TwoChar::BitshiftRight:	return "bitshift right";
		case TwoChar::Increment:		return "increment";
		case TwoChar::Decrement:		return "decrement";
		case TwoChar::StaticAccessor:	return "static accessor";
		case TwoChar::ArrayBrackets:	return "array brackets";
	}
}


const char* TwoCharToken::getSubTypeSymbol(unsigned short subType_) const
{
	switch ((TwoChar)subType_)
	{
		case TwoChar::Equals:			return "==";
		case TwoChar::NotEquals:		return "!=";
		case TwoChar::SmallerOrEqual:	return "<=";
		case TwoChar::GreaterOrEqual:	return ">=";
		case TwoChar::LogicalAnd:		return "&&";
		case TwoChar::LogicalOr:		return "||";
		case TwoChar::PlusAssign:		return "+=";
		case TwoChar::MinusAssign:		return "-=";
		case TwoChar::TimesAssign:		return "*=";
		case TwoChar::DivideAssign:		return "\\=";
		case TwoChar::BitwiseOrAssign:	return "|=";
		case TwoChar::BitwiseAndAssign:	return "&=";
		case TwoChar::BitwiseXorAssign:	return "^=";
		case TwoChar::BitshiftLeft:		return "<<";
		case TwoChar::BitshiftRight:	return ">>";
		case TwoChar::Increment:		return "++";
		case TwoChar::Decrement:		return "--";
		case TwoChar::StaticAccessor:	return "::";
		case TwoChar::ArrayBrackets:	return "[]";
		default:						return "??";
	}
		
}


bool TwoCharToken::createIfMatch(Document& document, std::size_t& currentPosition) const
{
	std::size_t remainingLength = document.getDocumentSize() - currentPosition;
	const char* currentCharacter = &document.getDocumentData()[currentPosition];
	if (remainingLength > 1)
	{
		std::string lex(currentCharacter, 2);
		TwoChar type = TwoChar::Unknown;
		
		if (lex == "==")			type = TwoChar::Equals;
		else if (lex == "!=")		type = TwoChar::NotEquals;
		else if (lex == "<=")		type = TwoChar::SmallerOrEqual;
		else if (lex == ">=")		type = TwoChar::GreaterOrEqual;
		else if (lex == "&&")		type = TwoChar::LogicalAnd;
		else if (lex == "||")		type = TwoChar::LogicalOr;
		else if (lex == "+=")		type = TwoChar::PlusAssign;
		else if (lex == "-=")		type = TwoChar::MinusAssign;
		else if (lex == "*=")		type = TwoChar::TimesAssign;
		else if (lex == "\\=")		type = TwoChar::DivideAssign;
		else if (lex == "|=")		type = TwoChar::BitwiseOrAssign;
		else if (lex == "&=")		type = TwoChar::BitwiseAndAssign;
		else if (lex == "^=")		type = TwoChar::BitwiseXorAssign;
		else if (lex == "<<")		type = TwoChar::BitshiftLeft;
		else if (lex == ">>")		type = TwoChar::BitshiftRight;
		else if (lex == "++")		type = TwoChar::Increment;
		else if (lex == "--")		type = TwoChar::Decrement;
		else if (lex == "::")		type = TwoChar::StaticAccessor;
		else if (lex == "[]")		type = TwoChar::ArrayBrackets;

		if (type != TwoChar::Unknown)
		{
			document.addToken(currentPosition, 2, id, Tools::enumToUSHort(type));
			currentPosition += 2;
			return true;
		}
	}
	return false;
}