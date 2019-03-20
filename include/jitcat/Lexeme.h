/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <iostream>
#include <string_view>


namespace jitcat::Tokenizer
{
	using Lexeme = std::string_view;

	//A lexeme points to a word or symbol contained in a document.
	//It is typically used to refer to the document text represented by a ParseToken.
	/*struct Lexeme
	{
		Lexeme(const std::string_view& lexeme);

		std::string_view lexeme;
	};

	bool operator==(const jitcat::Tokenizer::Lexeme& lexeme, const char* other);
	bool operator!=(const jitcat::Tokenizer::Lexeme& lexeme, const char* other);
	bool operator==(const jitcat::Tokenizer::Lexeme& lexeme, const std::string& other);
	bool operator!=(const jitcat::Tokenizer::Lexeme& lexeme, const std::string& other);

	std::ostream& operator<< (std::ostream& stream, const jitcat::Tokenizer::Lexeme* lexeme);*/

} //End namespace jitcat::Tokenizer
