/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <memory>
#include <vector>


namespace jitcat::Tokenizer
{
	class Document;
	class ParseToken;

	//The base class for a tokenizer. Contains functionality that is language-agnostic.
	//Tokens are generated by a token factory class. Multiple token factories can be added using the registerTokenFactory function.
	//A document can be tokenized using the tokenize function.
	class TokenizerBase
	{
	public:
		TokenizerBase();
		virtual ~TokenizerBase();

		//Returns true if the document was succesfully tokenized.
		//The tokens vector is filled with the parsed tokens.
		bool tokenize(Document* document, std::vector<std::unique_ptr<ParseToken>>& tokens, ParseToken* eofToken);
		void registerTokenFactory(std::unique_ptr<ParseToken> factory);
		const char* getTokenName(int tokenId, int tokenSubType) const;
		const char* getTokenSymbol(int tokenId, int tokenSubType) const;

		//Returns true if the token should be suggested if a parse error occurs and this token is in the follow set.
		bool isSuggestedToken(int tokenId, int tokenSubType) const;

	private:
		std::vector<std::unique_ptr<ParseToken>> tokenFactories;
	};

} //End namespace jitcat::Tokenizer