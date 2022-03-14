/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Lexeme.h"

namespace jitcat::Tokenizer
{
	class Document;


	struct ParseToken
	{
	public:
		ParseToken(const Lexeme& lexeme, unsigned short tokenID, unsigned short subType):
			lexeme(lexeme),
			tokenID(tokenID),
			subType(subType)
		{};

		const Lexeme lexeme;
		const unsigned short tokenID;
		const unsigned short subType;

		static const unsigned short eofType;
		static const ParseToken getEofToken(const Document& doc);
	};

	class TokenFactory
	{
	public:
		TokenFactory(unsigned short id): id(id){};
		unsigned short getTokenID() const {return id;};

		virtual const char* getTokenName() const = 0;
		virtual const char* getSubTypeName(unsigned short subType) const = 0;
		virtual const char* getSubTypeSymbol(unsigned short subType) const = 0;
		virtual bool createIfMatch(Document& document, std::size_t& currentPosition) const = 0;
		//Returns true if this token should be suggested when a parse error occurs and this token is in the follow set.
		virtual bool isSuggestedToken(unsigned short subType) const { return false; }

	protected:
		unsigned short id;
	};
} //End namespace jitcat::Tokenizer