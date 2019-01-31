/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "IdentifierToken.h"
#include "Document.h"
#include "Lexeme.h"
#include "ParseHelper.h"


IdentifierToken::IdentifierToken(Lexeme* lexeme_, Identifier subType):
	subType(subType)
{
	lexeme.reset(lexeme_);
}


int IdentifierToken::getTokenID() const 
{
	return getID();
}


const char* IdentifierToken::getTokenName() const 
{
	return "Identifier";
}


const char* IdentifierToken::getSubTypeName(int subType_) const
{
	switch ((Identifier) subType_)
	{
	default:
	case Identifier::Identifier:		return "identifier";
	case Identifier::Class:				return "class";
	case Identifier::Enum:				return "enum";
	case Identifier::Public:			return "public";
	case Identifier::Protected:			return "protected";
	case Identifier::Private:			return "private";
	case Identifier::Const:				return "const";
	case Identifier::Static:			return "static";
	case Identifier::Virtual:			return "virtual";
	case Identifier::If:				return "if";
	case Identifier::Then:				return "then";
	case Identifier::Else:				return "else";
	case Identifier::While:				return "while";
	case Identifier::Do:				return "do";
	case Identifier::For:				return "for";
	case Identifier::Continue:			return "continue";
	case Identifier::Break:				return "break";
	case Identifier::Switch:			return "switch";
	case Identifier::Case:				return "case";
	case Identifier::Default:			return "default";
	case Identifier::Return:			return "return";
	case Identifier::Void:				return "void";
	case Identifier::Unsigned:			return "unsigned";
	case Identifier::Char:				return "char";
	case Identifier::Int:				return "int";
	case Identifier::Long:				return "long";
	case Identifier::Float:				return "float";
	case Identifier::Double:			return "double";
	case Identifier::Vector4:			return "vector4";
	case Identifier::Matrix:			return "matrix";
	}
}


const char* IdentifierToken::getSubTypeSymbol(int subType_) const
{
	return getSubTypeName(subType_);
}


int IdentifierToken::getTokenSubType() const
{
	return (int)subType;
}


ParseToken* IdentifierToken::createIfMatch(Document* document, const char* currentPosition) const
{
	std::size_t offset = 0;
	std::size_t docOffset = currentPosition - document->getDocumentData();
	std::size_t documentLength = document->getDocumentSize() - docOffset;
	if (documentLength > 0)
	{
		if (ParseHelper::isAlphaNumeric(currentPosition[offset]))
		{
			offset++;
			while (offset < documentLength
				   && (   ParseHelper::isAlphaNumeric(currentPosition[offset])
					   || ParseHelper::isNumber(currentPosition[offset])
					   || currentPosition[offset] == '_')
					   /*|| currentPosition[offset] == '.')*/)
			{
				offset++;
			}
		}
	}
	if (offset > 0)
	{
		Lexeme* newLexeme = new Lexeme(document, docOffset, offset);
		for (int type = (int)Identifier::Class; type < (int)Identifier::Last; type++)
		{
			if (*newLexeme == getSubTypeName(type))
			{
				return new IdentifierToken(newLexeme, (Identifier)type);
			}
		}
		return new IdentifierToken(newLexeme, Identifier::Identifier);
	}
	else
	{
		return nullptr;
	}
}


const int IdentifierToken::getID()
{
	static int ID = ParseToken::getNextTokenID(); 
	return ID;
}