/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatTokenizer.h"
#include "jitcat/CommentToken.h"
#include "jitcat/ConstantToken.h"
#include "jitcat/ErrorToken.h"
#include "jitcat/IdentifierToken.h"
#include "jitcat/OneCharToken.h"
#include "jitcat/TwoCharToken.h"
#include "jitcat/WhitespaceToken.h"

using namespace jitcat::Tokenizer;


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