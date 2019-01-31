/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CommentToken.h"
#include "jitcat/ConstantToken.h"
#include "jitcat/ErrorToken.h"
#include "jitcat/IdentifierToken.h"
#include "jitcat/OneCharToken.h"
#include "jitcat/TwoCharToken.h"
#include "jitcat/WhitespaceToken.h"



namespace jitcat::Cat
{
	int comment = Tokenizer::CommentToken::getID();
	int ws		= Tokenizer::WhitespaceToken::getID();
	int lit		= Tokenizer::ConstantToken::getID();
	int id		= Tokenizer::IdentifierToken::getID();
	int err		= Tokenizer::ErrorToken::getID();
	int one		= Tokenizer::OneCharToken::getID();
	int two		= Tokenizer::TwoCharToken::getID();
};