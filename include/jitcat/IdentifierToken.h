/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ParseToken.h"
#include "jitcat/IdentifierTokenSubTypes.h"

namespace jitcat::Tokenizer
{

	class IdentifierToken: public ParseToken
	{
	public:
		IdentifierToken(): subType(Identifier::Identifier) {};
		IdentifierToken(const Lexeme& lexeme, Identifier subType);
		virtual int getTokenID() const;
		virtual const char* getTokenName() const;
		virtual const char* getSubTypeName(int subType) const;
		virtual const char* getSubTypeSymbol(int subType) const;
		virtual int getTokenSubType() const;
		virtual ParseToken* createIfMatch(Document* document, const char* currentPosition) const;
		static const int getID();

	private:
		Identifier subType;
	};

} //End namespace jitcat::Tokenizer