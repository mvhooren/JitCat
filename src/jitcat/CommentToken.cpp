/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CommentToken.h"
#include "jitcat/Document.h"
#include "jitcat/Lexeme.h"
#include "jitcat/Tools.h"

using namespace jitcat::Tokenizer;


const char* CommentToken::getSubTypeName(int subType_) const
{
	switch ((CommentType) subType_)
	{
		default:
		case CommentType::SingleLine:	return "single line";
		case CommentType::Block:		return "block";
	}
}


const char* CommentToken::getSubTypeSymbol(int subType_) const
{
	switch ((CommentType) subType_)
	{
		default:
		case CommentType::SingleLine:	return "//";
		case CommentType::Block:		return "/**/";
	}
}


ParseToken* CommentToken::createIfMatch(Document* document, const char* currentPosition) const
{
	std::size_t offset = 0;
	std::size_t docOffset = currentPosition - document->getDocumentData().c_str();
	std::size_t documentLength = document->getDocumentSize() - docOffset;
	CommentType subTypeToCreate = CommentType::SingleLine;
	if (documentLength >= 2)
	{
		if (currentPosition[0] == '/' && currentPosition[1] == '/')
		{
			offset += 2;
			while (offset < documentLength
				   && currentPosition[offset] != '\n')
			{
				offset++;
			}
		}
		else if (currentPosition[0] == '/' && currentPosition[1] == '*')
		{
			subTypeToCreate = CommentType::Block;
			offset += 2;
			bool previousCharacterIsStar = false;
			while (offset < documentLength
				   && (!previousCharacterIsStar || currentPosition[offset] != '/'))
			{
				previousCharacterIsStar = currentPosition[offset] == '*';
				offset++;
			}
			if (offset <= documentLength && currentPosition[offset] == '/' && previousCharacterIsStar)
			{
				offset++;
			}
		}
	}


	if (offset > 0)
	{
		return new CommentToken(document->createLexeme(docOffset, offset), Tools::enumToInt(subTypeToCreate));
	}
	else
	{
		return nullptr;
	}	
}


const int CommentToken::getID()
{
	static int ID = ParseToken::getNextTokenID(); 
	return ID;
}