/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "JitCat.h"

#include "CatASTNodes.h"
#include "CatGrammar.h"
#include "CatTokenizer.h"
#include "CommentToken.h"
#include "Document.h"
#include "IdentifierToken.h"
#include "Lexeme.h"
#include "OneCharToken.h"
#include "SLRParser.h"
#include "ParseToken.h"
#include "Tools.h"
#include "WhitespaceToken.h"

#include <string>
#include <vector>
#include <iostream>


JitCat::JitCat()
{
	tokenizer = new CatTokenizer();
	grammar = new CatGrammar(tokenizer);
	parser = grammar->createSLRParser();
}


JitCat::~JitCat()
{
	delete parser;
	delete grammar;
	delete tokenizer;
}


JitCat* JitCat::get()
{
	if (instance == nullptr)
	{
		instance = new JitCat();
	}
	return instance;
}


SLRParseResult* JitCat::parse(Document* expression, CatRuntimeContext* context) const
{
	std::vector<ParseToken*> tokens;
	OneCharToken* eofToken = new OneCharToken(new Lexeme(expression, expression->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(expression, tokens, eofToken);	
	SLRParseResult* result = parser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context);
	Tools::deleteElements(tokens);
	return result;
}


JitCat* JitCat::instance = nullptr;
