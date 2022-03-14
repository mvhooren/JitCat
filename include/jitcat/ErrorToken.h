/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ParseToken.h"

namespace jitcat::Tokenizer
{

	class ErrorToken: public TokenFactory
	{
	public:
		ErrorToken(unsigned short id): TokenFactory(id) {};
		virtual const char* getTokenName() const override final {return "Error";};
		virtual const char* getSubTypeName(unsigned short subType) const override final {return getTokenName();};	
		virtual const char* getSubTypeSymbol(unsigned short subType) const override final {return getTokenName();};	
		virtual bool createIfMatch(Document& document, std::size_t& currentPosition) const override final;
	};

} //End namespace jitcat::Tokenizer