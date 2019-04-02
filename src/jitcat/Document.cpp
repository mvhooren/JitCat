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
	document(document),
	currentLineIndex(0)
{
}


Document::Document(const char* fileData, std::size_t fileSize):
	document(fileData, fileSize),
	currentLineIndex(0)
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


std::tuple<int, int, int> jitcat::Tokenizer::Document::getLineColumnAndLength(const Lexeme& lexeme) const
{
	//Check that the lexeme lies inside the document
	assert((lexeme.data() >= document.c_str() 
			   && (lexeme.data() + lexeme.size()) <= document.c_str() + document.size()));
	int offset = (int)(lexeme.data() - document.c_str());
	auto& iter = lineNumberLookup.upper_bound(offset);
	if (iter != lineNumberLookup.end())
	{
		int lineNumber = iter->second;
		int lineStartOffset = 0;
		if (iter != lineNumberLookup.begin())
		{
			--iter;
			lineStartOffset = iter->first + 1;
		}
		int column = offset - lineStartOffset;
		return std::tuple<int, int, int>(lineNumber, column, (int)lexeme.length());
	}
	else
	{
		auto& iter = lineNumberLookup.rbegin();
		if (iter != lineNumberLookup.rend())
		{
			int lineNumber = iter->second;
			int lineStartOffset = 0;
			if (++iter != lineNumberLookup.rend())
			{
				lineStartOffset = iter->first + 1;
			}
			else
			{
				lineStartOffset = 0;
			}
			int column = offset - lineStartOffset;
			return std::tuple<int, int, int>(lineNumber, column, (int)lexeme.length());
		}
		return std::tuple<int, int, int>(0, 0, 0);
	}
}


int jitcat::Tokenizer::Document::offsetToLineNumber(int offset) const
{
	auto& iter = lineNumberLookup.upper_bound(offset);
	if (iter != lineNumberLookup.end())
	{
		return iter->second;
	}
	else
	{
		return 0;
	}
}


void jitcat::Tokenizer::Document::clearLineLookup()
{
	currentLineIndex = 0;
	lineNumberLookup.clear();
}


void jitcat::Tokenizer::Document::addNewLine(int offset)
{
	lineNumberLookup[offset] = currentLineIndex;
	currentLineIndex++;
}
