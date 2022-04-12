/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/IdentifierToken.h"
#include "jitcat/Document.h"
#include "jitcat/Lexeme.h"
#include "jitcat/ParseHelper.h"
#include "jitcat/Tools.h"

using namespace jitcat::Tokenizer;



const char* IdentifierToken::getTokenName() const 
{
	return "Identifier";
}


const char* IdentifierToken::getSubTypeName(unsigned short subType_) const
{
	switch ((Identifier) subType_)
	{
		default:
		case Identifier::Identifier:return "identifier";
		case Identifier::Class:		return "class";
		case Identifier::Struct:	return "struct";
		case Identifier::Inherits:	return "inherits";
		case Identifier::Enum:		return "enum";
		case Identifier::Public:	return "public";
		case Identifier::Protected:	return "protected";
		case Identifier::Private:	return "private";
		case Identifier::Const:		return "const";
		case Identifier::Static:	return "static";
		case Identifier::Virtual:	return "virtual";
		case Identifier::New:		return "new";
		case Identifier::If:		return "if";
		case Identifier::Then:		return "then";
		case Identifier::Else:		return "else";
		case Identifier::While:		return "while";
		case Identifier::Do:		return "do";
		case Identifier::For:		return "for";
		case Identifier::In:		return "in";
		case Identifier::Range:		return "range";
		case Identifier::Continue:	return "continue";
		case Identifier::Break:		return "break";
		case Identifier::Switch:	return "switch";
		case Identifier::Case:		return "case";
		case Identifier::Default:	return "default";
		case Identifier::Return:	return "return";
		case Identifier::Void:		return "void";
		case Identifier::Unsigned:	return "unsigned";
		case Identifier::Char:		return "char";
		case Identifier::Bool:		return "bool";
		case Identifier::Int:		return "int";
		case Identifier::Long:		return "long";
		case Identifier::Float:		return "float";
		case Identifier::Double:	return "double";
		case Identifier::Vector4f:	return "vector4f";
		case Identifier::Matrix4f:	return "matrix4f";
		case Identifier::Null:		return "null";
		case Identifier::Array:		return "array";
	}
}


const char* IdentifierToken::getSubTypeSymbol(unsigned short subType_) const
{
	return getSubTypeName(subType_);
}


bool IdentifierToken::createIfMatch(Document& document, std::size_t& currentPosition) const
{
	std::size_t offset = 0;
	std::size_t documentLength = document.getDocumentSize() - currentPosition;
	const char* currentCharacter = &document.getDocumentData()[currentPosition];
	if (documentLength > 0)
	{
		if (ParseHelper::isAlphaNumeric(currentCharacter[offset]) || currentCharacter[offset] == '_')
		{
			offset++;
			while (offset < documentLength
				   && (   ParseHelper::isAlphaNumeric(currentCharacter[offset])
					   || ParseHelper::isNumber(currentCharacter[offset])
					   || currentCharacter[offset] == '_')
					   /*|| currentPosition[offset] == '.')*/)
			{
				offset++;
			}
		}
	}
	if (offset > 0)
	{
		std::string idName(currentCharacter, offset);
		Identifier idType = Identifier::Identifier;
		for (unsigned short type = Tools::enumToUSHort(Identifier::Class); type < Tools::enumToUSHort(Identifier::Last); ++type)
		{
			if (idName == getSubTypeName(type))
			{
				idType = (Identifier)type;
			}
		}
		document.addToken(currentPosition, offset, id, Tools::enumToUSHort(idType));
		currentPosition += offset;
		return true;
	}
	else
	{
		return false;
	}
}