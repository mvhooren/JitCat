/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ParseToken.h"
#include "jitcat/Document.h"

#include <limits>

using namespace jitcat::Tokenizer;

const unsigned short ParseToken::eofType = std::numeric_limits<unsigned short>::max();

const ParseToken ParseToken::getEofToken(const Document& doc)
{
	return ParseToken(doc.createLexeme(doc.getDocumentSize(), 0), eofType, 0);;
}
