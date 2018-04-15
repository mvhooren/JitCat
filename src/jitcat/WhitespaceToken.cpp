/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "WhitespaceToken.h"
#include "Document.h"
#include "Lexeme.h"


ParseToken* WhitespaceToken::createIfMatch(Document* document, const char* currentPosition) const
{
	std::size_t offset = 0;
	std::size_t docOffset = currentPosition - document->getDocumentData();
	std::size_t documentLength = document->getDocumentSize() - docOffset;
	while (offset < documentLength
		   && (   currentPosition[offset] == ' '
			   || currentPosition[offset] == '\t'
			   || currentPosition[offset] == '\n'))
	{
		offset++;
	}
	if (offset > 0)
	{
		return new WhitespaceToken(new Lexeme(document, docOffset, offset));
	}
	else
	{
		return nullptr;
	}
}