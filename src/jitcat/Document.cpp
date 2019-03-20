/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Document.h"

#include <cassert>
#include <string.h>

using namespace jitcat::Tokenizer;


jitcat::Tokenizer::Document::Document(const std::string& document):
	document(document)
{
}


Document::Document(const char* fileData, std::size_t fileSize):
	document(fileData, fileSize)
{
}


Document::~Document()
{
}


const std::string& Document::getDocumentData() const
{
	return document;
}


std::size_t Document::getDocumentSize() const
{
	return document.size();
}


Lexeme jitcat::Tokenizer::Document::createLexeme(std::size_t offset, std::size_t length)
{
	if (offset + length <= document.size())
	{
		return Lexeme(document.c_str() + offset, length);
	}
	return Lexeme();
}


std::size_t jitcat::Tokenizer::Document::getOffsetInDocument(const Lexeme& lexeme) const
{
	//Check that the lexeme lies inside the document
	assert(lexeme.data() >= document.c_str() && (lexeme.data() + lexeme.size()) < document.c_str() + document.size());
	return lexeme.data() - document.c_str();
}
