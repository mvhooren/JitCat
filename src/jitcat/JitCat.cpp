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
#include "jitcat/Configuration.h"
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

#ifdef _WIN32
	extern "C" void _jc_enumerate_expressions(void(*enumeratorCallback)(const char*, uintptr_t));

	extern "C" void _jc_enumerate_expressions_default(void(*enumeratorCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_enumerate_expressions function implementation was found.
		enumeratorCallback("default", 0);
	}

	//Make sure the jc_enumerate_expression function is weakly linked to _jc_enumerate_expressions_default
	//Linking in a generated object file will override it with the generated _jc_enumerate_expressions function
	//This is MSVC only:
	#pragma comment(linker, "/alternatename:_jc_enumerate_expressions=_jc_enumerate_expressions_default")
#else
	extern "C" void _jc_enumerate_expressions(void(*enumeratorCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_enumerate_expressions function implementation was found.
		enumeratorCallback("default", 0);
	}	
#endif

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
	if constexpr (Configuration::usePreCompiledExpressions)
	{
		_jc_enumerate_expressions(&enumerationCallback);
	}
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


std::shared_ptr<PrecompilationContext> JitCat::createPrecompilationContext() const
{
	#ifdef ENABLE_LLVM
		return LLVM::LLVMJit::get().createLLVMPrecompilationContext();
	#else
		return nullptr;
	#endif
}


uintptr_t JitCat::getPrecompiledSymbol(const std::string& name)
{
	auto iter = precompiledSymbols.find(name);
	if (iter != precompiledSymbols.end())
	{
		return iter->second;
	}
	return 0;
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


void JitCat::enumerationCallback(const char * name, uintptr_t address)
{
	if (name == std::string("default")
		&& address == 0)
	{
		std::cout << "No precompiled expression symbols found." << std::endl;
	}
	else
	{
		precompiledSymbols.insert(std::make_pair(std::string(name), address));
	}
}


JitCat* JitCat::instance = nullptr;

std::unordered_map<std::string, uintptr_t> JitCat::precompiledSymbols = std::unordered_map<std::string, uintptr_t>();