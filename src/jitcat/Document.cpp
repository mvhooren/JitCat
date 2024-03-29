/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Document.h"

#include <cassert>
#include <algorithm>
#include <string.h>


using namespace jitcat::Tokenizer;


Document::Document():
	currentLineIndex(0)
{
}


Document::Document(const std::string& document):
	document(document),
	currentLineIndex(0)
{
}


Document::Document(const char* fileData, std::size_t fileSize):
	document(fileData, fileSize),
	currentLineIndex(0)
{
}


Document::Document(Document&& other) noexcept:
	document(std::move(other.document)),
	lineNumberLookup(std::move(other.lineNumberLookup)),
	currentLineIndex(other.currentLineIndex),
	tokens(std::move(other.tokens))
{
}


Document::~Document()
{
}


Document& Document::operator=(Document&& other) noexcept
{
	document = std::move(other.document);
	lineNumberLookup = std::move(other.lineNumberLookup);
	currentLineIndex = other.currentLineIndex;
	tokens = std::move(other.tokens);
	return *this;
}


const std::string& Document::getDocumentData() const
{
	return document;
}


std::size_t Document::getDocumentSize() const
{
	return document.size();
}


Lexeme Document::createLexeme(std::size_t offset, std::size_t length) const
{
	if (offset + length <= document.size())
	{
		return Lexeme(document.c_str() + offset, length);
	}
	return Lexeme();
}


std::size_t Document::getOffsetInDocument(const Lexeme& lexeme) const
{
	//Check that the lexeme lies inside the document
	if (lexeme.data() >= document.c_str() && (lexeme.data() + lexeme.size()) <= document.c_str() + document.size())
	{
		return lexeme.data() - document.c_str();
	}
	else
	{
		return 0;
	}
}


DocumentSelection Document::toSelection(const Lexeme& lexeme) const
{
	//Check that the lexeme lies inside the document
	assert((lexeme.data() >= document.c_str() 
			   && (lexeme.data() + lexeme.size()) <= document.c_str() + document.size()));
	int offset = (int)(lexeme.data() - document.c_str());
	auto[startLine, startColumn] = getLineAndColumnNumber(offset);
	auto[endLine, endColumn] = getLineAndColumnNumber(offset + (int)lexeme.length());
	return DocumentSelection(startLine, startColumn, endLine, endColumn);
}


std::tuple<int, int> Document::getLineAndColumnNumber(const Lexeme & lexeme) const
{
	assert((lexeme.data() >= document.c_str() 
			   && (lexeme.data() + lexeme.size()) <= document.c_str() + document.size()));
	int offset = (int)(lexeme.data() - document.c_str());
	return getLineAndColumnNumber(offset);
}


std::tuple<int, int> Document::getLineAndColumnNumber(int offset) const
{
	auto iter = lineNumberLookup.upper_bound(offset);
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
		return std::tuple<int, int>(lineNumber, column);
	}
	else
	{
		auto iter = lineNumberLookup.rbegin();
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
			return std::tuple<int, int>(lineNumber, column);
		}
		return std::tuple<int, int>(0, 0);
	}
}


int Document::offsetToLineNumber(int offset) const
{
	auto iter = lineNumberLookup.upper_bound(offset);
	if (iter != lineNumberLookup.end())
	{
		return iter->second;
	}
	else
	{
		return 0;
	}
}


void Document::clearLineLookup()
{
	currentLineIndex = 0;
	lineNumberLookup.clear();
}


void Document::addNewLine(int offset)
{
	lineNumberLookup[offset] = currentLineIndex;
	currentLineIndex++;
}


bool Document::isValidLexeme(const Lexeme& lexeme) const
{
	return  lexeme.data() >= document.c_str() 
			 && (lexeme.data() + lexeme.size()) <= document.c_str() + document.size();
}


void Document::clearTokens()
{
	tokens.clear();
}


void Document::addToken(std::size_t offset, std::size_t length, unsigned short tokenID, unsigned short subType)
{
	tokens.emplace_back(createLexeme(offset, length), tokenID, subType);
}


void Document::addToken(ParseToken token)
{
	tokens.emplace_back(std::move(token));
}


const std::vector<ParseToken>& Document::getTokens() const
{
	return tokens;
}
