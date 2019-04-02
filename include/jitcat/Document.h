/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "Lexeme.h"
#include <map>
#include <string>
#include <tuple>


namespace jitcat::Tokenizer
{
	class Document
	{
	public: 
		Document(const std::string& document);
		Document(const char* fileData, std::size_t fileSize);
		~Document();
		const std::string& getDocumentData() const;
		std::size_t getDocumentSize() const;
		Lexeme createLexeme(std::size_t offset, std::size_t length);

		std::size_t getOffsetInDocument(const Lexeme& lexeme) const;
		std::tuple<int, int, int> getLineColumnAndLength(const Lexeme& lexeme) const;
		
		int offsetToLineNumber(int offset) const;
		void clearLineLookup();
		void addNewLine(int offset);


	private:
		std::string document;
		//Stores a map from offsets to line numbers. 
		std::map<int, int> lineNumberLookup;
		int currentLineIndex;
	};

} //End namespace jitcat::Tokenizer