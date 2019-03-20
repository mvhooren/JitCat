/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ParseToken.h"
#include "jitcat/OneCharTokenSubTypes.h"

namespace jitcat::Tokenizer
{
	class Document;

	class OneCharToken: public ParseToken
	{
	public:
		OneCharToken();
		OneCharToken(const Lexeme& lexeme, OneChar subType);
		virtual ~OneCharToken();
		virtual int getTokenID() const;
		virtual const char* getTokenName() const;
		virtual const char* getSubTypeName(int subType) const;
		virtual const char* getSubTypeSymbol(int subType) const;
		virtual int getTokenSubType() const;
		virtual ParseToken* createIfMatch(Document* document, const char* currentPosition) const;
		static const int getID();
	private:
		OneChar subType;
	};

} //End namespace jitcat::Tokenizer