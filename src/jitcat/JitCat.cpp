/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/JitCat.h"

#include "jitcat/CatASTNodes.h"
#include "jitcat/CatGrammar.h"
#include "jitcat/CatTokenizer.h"
#include "jitcat/CommentToken.h"
#include "jitcat/Document.h"
#include "jitcat/IdentifierToken.h"
#include "jitcat/Lexeme.h"
#include "jitcat/OneCharToken.h"
#include "jitcat/SLRParser.h"
#include "jitcat/ParseToken.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"
#include "jitcat/WhitespaceToken.h"
#ifdef ENABLE_LLVM
#include "jitcat/LLVMJit.h"
#endif
#include <string>
#include <vector>
#include <iostream>

using namespace jitcat;
using namespace jitcat::Grammar;
using namespace jitcat::LLVM;
using namespace jitcat::Parser;
using namespace jitcat::Reflection;
using namespace jitcat::Tokenizer;

JitCat::JitCat():
	tokenizer(new CatTokenizer()),
	expressionGrammar(new CatGrammar(tokenizer.get(), CatGrammarType::Expression)),
	fullGrammar(new CatGrammar(tokenizer.get(), CatGrammarType::Full))
{
	expressionParser = expressionGrammar->createSLRParser();
	fullParser = fullGrammar->createSLRParser();
}


JitCat::~JitCat()
{
}


JitCat* JitCat::get()
{
	if (instance == nullptr)
	{
		instance = new JitCat();
	}
	return instance;
}


Parser::SLRParseResult* jitcat::JitCat::parseExpression(Tokenizer::Document* expression, CatRuntimeContext* context) const
{
	std::vector<ParseToken*> tokens;
	OneCharToken* eofToken = new OneCharToken(new Lexeme(expression, expression->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(expression, tokens, eofToken);	
	SLRParseResult* result = expressionParser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context);
	Tools::deleteElements(tokens);
	return result;
}


Parser::SLRParseResult* jitcat::JitCat::parseFull(Tokenizer::Document* expression, CatRuntimeContext* context) const
{
	std::vector<ParseToken*> tokens;
	OneCharToken* eofToken = new OneCharToken(new Lexeme(expression, expression->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(expression, tokens, eofToken);	
	SLRParseResult* result = fullParser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context);
	Tools::deleteElements(tokens);
	return result;
}


void jitcat::JitCat::destroy()
{
	delete instance;
	instance = nullptr;
	TypeRegistry::get()->recreate();
	#ifdef ENABLE_LLVM
		LLVMJit::get().cleanup();
	#endif
}


JitCat* JitCat::instance = nullptr;
