/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatTokenizer.h"
#include "CommentToken.h"
#include "ConstantToken.h"
#include "ErrorToken.h"
#include "IdentifierToken.h"
#include "OneCharToken.h"
#include "TwoCharToken.h"
#include "WhitespaceToken.h"



CatTokenizer::CatTokenizer()
{
	//The order of these matter because they determine which token gets matched first if there are multiple tokens that can create a match.
	registerTokenFactory(new WhitespaceToken());
	registerTokenFactory(new CommentToken());
	registerTokenFactory(new ConstantToken());
	registerTokenFactory(new IdentifierToken());
	registerTokenFactory(new TwoCharToken());
	registerTokenFactory(new OneCharToken());
	registerTokenFactory(new ErrorToken()); //Should be last, matches all but whitespace
}