/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Lexeme.h"

#include <memory>


namespace jitcat::Tokenizer
{
	class Document;


	class ParseToken
	{
	public:
		ParseToken();
		ParseToken(const Lexeme& lexeme);
		virtual ~ParseToken();
		virtual int getTokenID() const = 0;
		virtual const char* getTokenName() const = 0;
		virtual const char* getSubTypeName(int subType) const = 0;	
		virtual const char* getSubTypeSymbol(int subType) const = 0;
		virtual int getTokenSubType() const = 0;
		const Lexeme& getLexeme() const;
		virtual ParseToken* createIfMatch(Document* document, const char* currentPosition) const = 0;

		//Returns true if this token should be suggested when a parse error occurs and this token is in the follow set.
		virtual bool isSuggestedToken(int subType) const {return false;}

	protected:	
		static int getNextTokenID();
	protected:
		Lexeme lexeme;
	};

} //End namespace jitcat::Tokenizer