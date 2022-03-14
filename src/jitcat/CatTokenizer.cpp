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
	registerTokenFactory(std::make_unique<WhitespaceToken>(whiteSpace));
	registerTokenFactory(std::make_unique<CommentToken>(comment));
	registerTokenFactory(std::make_unique<ConstantToken>(constant));
	registerTokenFactory(std::make_unique<IdentifierToken>(identifier));
	registerTokenFactory(std::make_unique<TwoCharToken>(twoChar));
	registerTokenFactory(std::make_unique<OneCharToken>(oneChar));
	registerTokenFactory(std::make_unique<ErrorToken>(error)); //Should be last, matches all but whitespace
}


const unsigned short CatTokenizer::whiteSpace = 0;
const unsigned short CatTokenizer::comment = 1;
const unsigned short CatTokenizer::constant = 2;
const unsigned short CatTokenizer::identifier = 3;
const unsigned short CatTokenizer::twoChar = 4;
const unsigned short CatTokenizer::oneChar = 5;
const unsigned short CatTokenizer::error = 6;
