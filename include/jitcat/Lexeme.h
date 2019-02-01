/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <iostream>
#include <string.h>

namespace jitcat::Tokenizer
{
	class Document;

	//A lexeme points to a word or symbol contained in a document.
	//It is typically used to refer to the document text represented by a ParseToken.
	struct Lexeme
	{
		Lexeme(Document* document, std::size_t offset, std::size_t length);
		//Get a pointer to the contents of the document referred to by this Lexeme.
		//The const char* is valid as long as the document is not edited.
		const char* getDataPointer() const;

		Document* document;
		std::size_t offset;
		std::size_t length;

		std::string toString() const;

	};

	bool operator==(const jitcat::Tokenizer::Lexeme& lexeme, const char* other);
	bool operator!=(const jitcat::Tokenizer::Lexeme& lexeme, const char* other);
	bool operator==(const jitcat::Tokenizer::Lexeme& lexeme, const std::string& other);
	bool operator!=(const jitcat::Tokenizer::Lexeme& lexeme, const std::string& other);

	std::ostream& operator<< (std::ostream& stream, const jitcat::Tokenizer::Lexeme* lexeme);

} //End namespace jitcat::Tokenizer
