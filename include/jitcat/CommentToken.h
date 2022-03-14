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

class CommentToken: public TokenFactory
{
public:
	CommentToken(unsigned short tokenID): TokenFactory(tokenID) {};
	virtual const char* getTokenName() const override final {return "Comment";};
	virtual const char* getSubTypeName(unsigned short subType) const override final;
	virtual const char* getSubTypeSymbol(unsigned short subType) const override final;
	virtual bool createIfMatch(Document& document, std::size_t& currentPosition) const override final;
};


} //End namespace jitcat::Tokenizer