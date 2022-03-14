/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/WhitespaceToken.h"
#include "jitcat/Document.h"
#include "jitcat/Lexeme.h"

using namespace jitcat::Tokenizer;


bool WhitespaceToken::createIfMatch(Document& document, std::size_t& currentPosition) const
{
	std::size_t offset = 0;
	std::size_t documentLength = document.getDocumentSize() - currentPosition;
	const char* currentCharacter = &document.getDocumentData()[currentPosition];
	bool seenCarriageReturn = false;
	while (offset < documentLength
		   && (	  currentCharacter[offset] == ' '
			   || currentCharacter[offset] == '\t'
			   || currentCharacter[offset] == '\n'
			   || currentCharacter[offset] == '\r'))
	{
		if (currentCharacter[offset] == '\r' && !seenCarriageReturn)
		{
			seenCarriageReturn = true;
		}
		else if (currentCharacter[offset] == '\r' || currentCharacter[offset] == '\n')
		{
			seenCarriageReturn = false;
			document.addNewLine((int)(currentPosition + offset));
		}
		offset++;
		
	}
	if (offset > 0)
	{
		document.addToken(currentPosition, offset, id, 0);
		currentPosition += offset;
		return true;
	}
	else
	{
		return false;
	}
}