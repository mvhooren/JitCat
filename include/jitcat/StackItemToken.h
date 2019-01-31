/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/StackItem.h"

namespace jitcat::Parser
{

	class StackItemToken : public StackItem
	{
	public:
		StackItemToken(Tokenizer::ParseToken* token):
			token(token) {}
			virtual const Tokenizer::ParseToken* getTokenIfToken() const {return token;}
	private:
		Tokenizer::ParseToken* token;
	};

} //End namespace jitcat::Parser