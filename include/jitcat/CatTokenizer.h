/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/TokenizerBase.h"

namespace jitcat::Tokenizer
{
	//Defines the tokens that make up the JitCat language.
	class CatTokenizer : public TokenizerBase
	{
	public:
		CatTokenizer();
		static const unsigned short whiteSpace;
		static const unsigned short comment;
		static const unsigned short constant;
		static const unsigned short identifier;
		static const unsigned short twoChar;
		static const unsigned short oneChar;
		static const unsigned short error;
	};

} //End namespace jitcat