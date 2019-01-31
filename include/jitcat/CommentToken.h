/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CommentTokenSubTypes.h"
#include "jitcat/ParseToken.h"

namespace jitcat::Tokenizer
{

class CommentToken: public ParseToken
{
public:
	CommentToken() {};
	CommentToken(Lexeme* lexeme_, int subType): subType(subType) {lexeme.reset(lexeme_);};
	virtual int getTokenID() const {return getID();};
	virtual const char* getTokenName() const {return "Comment";};
	virtual const char* getSubTypeName(int subType) const;
	virtual const char* getSubTypeSymbol(int subType) const;
	virtual int getTokenSubType() const {return subType;};
	virtual ParseToken* createIfMatch(Document* document, const char* currentPosition) const;

	static const int getID();

private:
	int subType;
};


} //End namespace jitcat::Tokenizer