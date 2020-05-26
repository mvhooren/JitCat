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
#include <time.h>
#include <vector>
#include <iostream>

using namespace jitcat;
using namespace jitcat::Grammar;
using namespace jitcat::LLVM;
using namespace jitcat::Parser;
using namespace jitcat::Reflection;
using namespace jitcat::Tokenizer;

JitCat::JitCat():
	tokenizer(std::make_unique<CatTokenizer>()),
	expressionGrammar(std::make_unique<CatGrammar>(tokenizer.get(), CatGrammarType::Expression)),
	statementGrammar(std::make_unique<CatGrammar>(tokenizer.get(), CatGrammarType::Statement)),
	fullGrammar(std::make_unique<CatGrammar>(tokenizer.get(), CatGrammarType::Full))
{
	expressionParser = expressionGrammar->createSLRParser();
	statementParser = statementGrammar->createSLRParser();
	fullParser = fullGrammar->createSLRParser();
	std::srand((unsigned int)time(nullptr));
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


std::unique_ptr<Parser::SLRParseResult> jitcat::JitCat::parseExpression(Tokenizer::Document* expression, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const
{
	std::vector<std::unique_ptr<ParseToken>> tokens;
	OneCharToken* eofToken = new OneCharToken(expression->createLexeme(expression->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(expression, tokens, eofToken);
	return expressionParser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context, errorManager, errorContext);
}


std::unique_ptr<Parser::SLRParseResult> jitcat::JitCat::parseStatement(Tokenizer::Document* statement, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const
{
	std::vector<std::unique_ptr<ParseToken>> tokens;
	OneCharToken* eofToken = new OneCharToken(statement->createLexeme(statement->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(statement, tokens, eofToken);
	return statementParser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context, errorManager, errorContext);
}


std::unique_ptr<Parser::SLRParseResult> jitcat::JitCat::parseFull(Tokenizer::Document* expression, std::vector<std::unique_ptr<Tokenizer::ParseToken>>& tokens, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const
{
	OneCharToken* eofToken = new OneCharToken(expression->createLexeme(expression->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(expression, tokens, eofToken);	
	return fullParser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context, errorManager, errorContext);
}


std::unique_ptr<Parser::SLRParseResult> jitcat::JitCat::parseFull(Tokenizer::Document* expression, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const
{
	std::vector<std::unique_ptr<ParseToken>> tokens;
	OneCharToken* eofToken = new OneCharToken(expression->createLexeme(expression->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(expression, tokens, eofToken);	
	return fullParser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context, errorManager, errorContext);
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
