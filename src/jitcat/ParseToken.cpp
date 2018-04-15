/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ParseToken.h"
#include "Lexeme.h"


ParseToken::ParseToken():
	lexeme(0)
{
}


ParseToken::~ParseToken()
{
	delete lexeme;
}


const Lexeme* ParseToken::getLexeme() const
{
	return lexeme;
}


int ParseToken::getNextTokenID()
{
	return nextTokenID++;
}


int ParseToken::nextTokenID = 0;
