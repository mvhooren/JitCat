/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ParseToken.h"

namespace jitcat::Tokenizer
{

	class ErrorToken: public ParseToken
	{
	public:
		ErrorToken() {};
		ErrorToken(Lexeme* lexeme_) {lexeme.reset(lexeme_);};
		virtual int getTokenID() const {return getID();};
		virtual const char* getTokenName() const {return "Error";};
		virtual const char* getSubTypeName(int subType) const {return getTokenName();};	
		virtual const char* getSubTypeSymbol(int subType) const {return getTokenName();};	
		virtual int getTokenSubType() const {return 0;};
		virtual ParseToken* createIfMatch(Document* document, const char* currentPosition) const;

		static const int getID(){static int ID = ParseToken::getNextTokenID(); return ID;};
	};

} //End namespace jitcat::Tokenizer