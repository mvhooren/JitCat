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

	extern "C" void _jc_enumerate_global_scopes(void(*enumeratorCallback)(const char*, uintptr_t));

	extern "C" void _jc_enumerate_global_scopes_default(void(*enumeratorCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_enumerate_expressions function implementation was found.
		enumeratorCallback("default", 0);
	}


	//Make sure these functions are weakly linked to their default alternatives
	//Linking in a generated object file will override the weakly linked symbol.
	//This is MSVC only:
	#pragma comment(linker, "/alternatename:_jc_enumerate_expressions=_jc_enumerate_expressions_default")
	#pragma comment(linker, "/alternatename:_jc_enumerate_global_scopes=_jc_enumerate_global_scopes_default")
#else
	extern "C" void _jc_enumerate_expressions(void(*enumeratorCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_enumerate_expressions function implementation was found.
		enumeratorCallback("default", 0);
	}	

	extern "C" void _jc_enumerate_global_scopes(void(*enumeratorCallback)(const char*, uintptr_t))
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
		_jc_enumerate_expressions(&expressionEnumerationCallback);
		_jc_enumerate_global_scopes(&globalScopesEnumerationCallback);
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
	auto iter = precompiledExpressionSymbols.find(name);
	if (iter != precompiledExpressionSymbols.end())
	{
		return iter->second;
	}
	return 0;
}


bool JitCat::setPrecompiledGlobalScopeVariable(const std::string_view variableName, unsigned char* value)
{
	auto iter = precompiledGlobalScopeVariables.find(variableName);
	if (iter != precompiledGlobalScopeVariables.end())
	{
		uintptr_t variableAddress = iter->second;
		unsigned char** variablePtr = reinterpret_cast<unsigned char**>(variableAddress);
		*variablePtr = value;
		return true;
	}
	return false;
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


std::string_view JitCat::defineGlobalScopeName(const std::string& globalName)
{
	auto iter = globalNames.find(globalName);
	if (iter != globalNames.end())
	{
		return *iter;
	}
	else
	{
		globalNames.insert(globalName);
		auto iter = globalNames.find(globalName);
		return *iter;
	}
}


void JitCat::expressionEnumerationCallback(const char* name, uintptr_t address)
{
	if (name == std::string("default")
		&& address == 0)
	{
		std::cout << "No precompiled expression symbols found." << std::endl;
	}
	else
	{
		precompiledExpressionSymbols.insert(std::make_pair(std::string(name), address));
	}
}


void JitCat::globalScopesEnumerationCallback(const char* name, uintptr_t address)
{
	if (name == std::string("default")
		&& address == 0)
	{
		std::cout << "No global scope symbols found." << std::endl;
	}
	else
	{
		precompiledGlobalScopeVariables.insert(std::make_pair(defineGlobalScopeName(name), address));
	}
}


JitCat* JitCat::instance = nullptr;

std::unordered_map<std::string, uintptr_t> JitCat::precompiledExpressionSymbols = std::unordered_map<std::string, uintptr_t>();
std::unordered_map<std::string_view, uintptr_t> JitCat::precompiledGlobalScopeVariables = std::unordered_map<std::string_view, uintptr_t>();
std::unordered_set<std::string> JitCat::globalNames = std::unordered_set<std::string>();