/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ErrorToken.h"
#include "jitcat/Document.h"
#include "jitcat/Lexeme.h"

using namespace jitcat::Tokenizer;


bool ErrorToken::createIfMatch(Document& document, std::size_t& currentPosition) const
{
	std::size_t offset = 0;
	std::size_t documentLength = document.getDocumentSize() - currentPosition;
	const char* currentCharacter = &document.getDocumentData()[currentPosition];
	while (offset < documentLength
		   && currentCharacter[offset] != ' '
		   && currentCharacter[offset] != '\t'
		   && currentCharacter[offset] != '\n')
	{
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