/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ParseToken.h"
#include "jitcat/Lexeme.h"

using namespace jitcat::Tokenizer;


ParseToken::ParseToken()
{
}


jitcat::Tokenizer::ParseToken::ParseToken(const Lexeme& lexeme):
	lexeme(lexeme)
{
}


ParseToken::~ParseToken()
{
}


const Lexeme& ParseToken::getLexeme() const
{
	return lexeme;
}


int ParseToken::getNextTokenID()
{
	return nextTokenID++;
}


int ParseToken::nextTokenID = 0;
