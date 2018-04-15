/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "Lexeme.h"
#include "Document.h"

Lexeme::Lexeme(Document* document, std::size_t offset, std::size_t length):
	document(document),
	offset(offset),
	length(length)
{
}

const char* Lexeme::getDataPointer() const
{
	if (document->getDocumentSize() >= offset + length)
	{
		return document->getDocumentData() + offset;
	}
	else
	{
		return 0;
	}
}

