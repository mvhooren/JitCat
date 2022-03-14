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


const char* CommentToken::getSubTypeName(unsigned short subType_) const
{
	switch ((CommentType) subType_)
	{
		default:
		case CommentType::SingleLine:	return "single line";
		case CommentType::Block:		return "block";
	}
}


const char* CommentToken::getSubTypeSymbol(unsigned short subType_) const
{
	switch ((CommentType) subType_)
	{
		default:
		case CommentType::SingleLine:	return "//";
		case CommentType::Block:		return "/**/";
	}
}


bool CommentToken::createIfMatch(Document& document, std::size_t& currentPosition) const
{
	std::size_t offset = 0;
	std::size_t documentLength = document.getDocumentSize() - currentPosition;
	const char* currentCharacter = &document.getDocumentData()[currentPosition];
	CommentType subTypeToCreate = CommentType::SingleLine;
	if (documentLength >= 2)
	{
		if (currentCharacter[0] == '/' && currentCharacter[1] == '/')
		{
			offset += 2;
			while (offset < documentLength
				   && currentCharacter[offset] != '\n')
			{
				offset++;
			}
		}
		else if (currentCharacter[0] == '/' && currentCharacter[1] == '*')
		{
			subTypeToCreate = CommentType::Block;
			offset += 2;
			bool previousCharacterIsStar = false;
			while (offset < documentLength
				   && (!previousCharacterIsStar || currentCharacter[offset] != '/'))
			{
				previousCharacterIsStar = currentCharacter[offset] == '*';
				offset++;
			}
			if (offset <= documentLength && currentCharacter[offset] == '/' && previousCharacterIsStar)
			{
				offset++;
			}
		}
	}


	if (offset > 0)
	{
		document.addToken(currentPosition, offset, id, Tools::enumToUSHort(subTypeToCreate));
		currentPosition += offset;
		return true;
	}
	else
	{
		return false;
	}	
}