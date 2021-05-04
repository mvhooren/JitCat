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
#include "jitcat/LLVMCatIntrinsics.h"
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

	extern "C" void _jc_enumerate_global_variables(void(*enumeratorCallback)(const char*, uintptr_t));

	extern "C" void _jc_enumerate_global_variables_default(void(*enumeratorCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_enumerate_global_scopes function implementation was found.
		enumeratorCallback("default", 0);
	}

	extern "C" void _jc_enumerate_linked_functions(void(*enumeratorCallback)(const char*, uintptr_t));

	extern "C" void _jc_enumerate_linked_functions_default(void(*enumeratorCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_enumerate_linked_functions function implementation was found.
		enumeratorCallback("default", 0);
	}
	
	extern "C" void _jc_initialize_string_pool(void(*stringInitializerCallback)(const char*, uintptr_t));

	extern "C" void _jc_initialize_string_pool_default(void(*stringInitializerCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_initialize_string_pool function implementation was found.
		stringInitializerCallback("default", 0);
	}

	//Make sure these functions are weakly linked to their default alternatives
	//Linking in a generated object file will override the weakly linked symbol.
	//This is MSVC only:
	#pragma comment(linker, "/alternatename:_jc_enumerate_expressions=_jc_enumerate_expressions_default")
	#pragma comment(linker, "/alternatename:_jc_enumerate_global_variables=_jc_enumerate_global_variables_default")
	#pragma comment(linker, "/alternatename:_jc_enumerate_linked_functions=_jc_enumerate_linked_functions_default")
	#pragma comment(linker, "/alternatename:_jc_initialize_string_pool=_jc_initialize_string_pool_default")
#else
	extern "C" void _jc_enumerate_expressions(void(*enumeratorCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_enumerate_expressions function implementation was found.
		enumeratorCallback("default", 0);
	}	

	extern "C" void _jc_enumerate_global_variables(void(*enumeratorCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_enumerate_global_variables function implementation was found.
		enumeratorCallback("default", 0);
	}

	extern "C" void _jc_enumerate_linked_functions(void(*enumeratorCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_enumerate_linked_functions function implementation was found.
		enumeratorCallback("default", 0);
	}
	
	extern "C" void _jc_initialize_string_pool(void(*stringInitializerCallback)(const char*, uintptr_t))
	{
		//Notify the callback that no proper _jc_initialize_string_pool function implementation was found.
		stringInitializerCallback("default", 0);		
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
		_jc_initialize_string_pool(&stringPoolInitializationCallback);
		_jc_enumerate_expressions(&expressionEnumerationCallback);
		_jc_enumerate_global_variables(&globalVariablesEnumerationCallback);
		_jc_enumerate_linked_functions(&linkedFunctionsEnumerationCallback);

		//Link in some of the JitCat std-lib functions that can't be linked in using extern "C".
		setPrecompiledLinkedFunction("boolToString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::boolToString));
		setPrecompiledLinkedFunction("doubleToString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::doubleToString));
		setPrecompiledLinkedFunction("floatToString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::floatToString));
		setPrecompiledLinkedFunction("intToString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::intToString));
		setPrecompiledLinkedFunction("uIntToString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::uIntToString));
		setPrecompiledLinkedFunction("int64ToString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::int64ToString));
		setPrecompiledLinkedFunction("uInt64ToString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::uInt64ToString));
		setPrecompiledLinkedFunction("intToPrettyString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::intToPrettyString));
		setPrecompiledLinkedFunction("intToFixedLengthString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::intToFixedLengthString));
		setPrecompiledLinkedFunction("roundFloatToString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::roundFloatToString));
		setPrecompiledLinkedFunction("roundDoubleToString", reinterpret_cast<uintptr_t>(&LLVMCatIntrinsics::roundDoubleToString));
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


std::unique_ptr<Parser::SLRParseResult> JitCat::parseExpression(Tokenizer::Document* expression, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const
{
	std::vector<std::unique_ptr<ParseToken>> tokens;
	OneCharToken* eofToken = new OneCharToken(expression->createLexeme(expression->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(expression, tokens, eofToken);
	return expressionParser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context, errorManager, errorContext);
}


std::unique_ptr<Parser::SLRParseResult> JitCat::parseStatement(Tokenizer::Document* statement, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const
{
	std::vector<std::unique_ptr<ParseToken>> tokens;
	OneCharToken* eofToken = new OneCharToken(statement->createLexeme(statement->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(statement, tokens, eofToken);
	return statementParser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context, errorManager, errorContext);
}


std::unique_ptr<Parser::SLRParseResult> JitCat::parseFull(Tokenizer::Document* expression, std::vector<std::unique_ptr<Tokenizer::ParseToken>>& tokens, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const
{
	OneCharToken* eofToken = new OneCharToken(expression->createLexeme(expression->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(expression, tokens, eofToken);	
	return fullParser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context, errorManager, errorContext);
}


std::unique_ptr<Parser::SLRParseResult> JitCat::parseFull(Tokenizer::Document* expression, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const
{
	std::vector<std::unique_ptr<ParseToken>> tokens;
	OneCharToken* eofToken = new OneCharToken(expression->createLexeme(expression->getDocumentSize(), 0), OneChar::Eof);
	tokenizer->tokenize(expression, tokens, eofToken);	
	return fullParser->parse(tokens, WhitespaceToken::getID(), CommentToken::getID(), context, errorManager, errorContext);
}


uintptr_t JitCat::getPrecompiledSymbol(const std::string& name)
{
	auto& precompiledExpressionSymbols = getPrecompiledExpressionSymbols();
	auto iter = precompiledExpressionSymbols.find(name);
	if (iter != precompiledExpressionSymbols.end())
	{
		return iter->second;
	}
	return 0;
}


bool JitCat::setPrecompiledGlobalVariable(const std::string_view variableName, unsigned char* value)
{
	return setPrecompiledGlobalVariable(variableName, reinterpret_cast<uintptr_t>(value));
}


bool JitCat::setPrecompiledGlobalVariable(const std::string_view variableName, uintptr_t value)
{
	auto& precompiledGlobalVariables = getPrecompiledGlobalVariables();
	auto iter = precompiledGlobalVariables.find(variableName);
	if (iter != precompiledGlobalVariables.end())
	{
		uintptr_t variableAddress = iter->second;
		uintptr_t* variablePtr = reinterpret_cast<uintptr_t*>(variableAddress);
		*variablePtr = value;
		return true;
	} 
	return false;
}


bool JitCat::setPrecompiledLinkedFunction(const std::string mangledFunctionName, uintptr_t address)
{
	auto& precompiledLinkedFunctions = getPrecompiledLinkedFunctions();
	auto iter = precompiledLinkedFunctions.find(mangledFunctionName);
	if (iter != precompiledLinkedFunctions.end())
	{
		uintptr_t* functionPtrPtr = reinterpret_cast<uintptr_t*>(iter->second);
		*functionPtrPtr = address;
		return true;
	}
	return false;
}


void JitCat::destroy()
{
	delete instance;
	instance = nullptr;
	TypeRegistry::get()->recreate();
	#ifdef ENABLE_LLVM
		LLVMJit::get().cleanup();
	#endif
}


std::string_view JitCat::defineGlobalVariableName(const std::string& globalName)
{
	auto& globalNames = getGlobalNames();
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


bool JitCat::verifyLinkage()
{
	bool verifySuccess = true;
	std::size_t correctLinkCount = 0;
	std::size_t incorrectLinkCount = 0;
	std::cout << "Verifying JitCat function linkage...\n";
	for (auto iter : getPrecompiledLinkedFunctions())
	{
		if (*reinterpret_cast<uintptr_t*>(iter.second) == 0)
		{
			std::cout << "JitCat linkage verification error: linked function " << iter.first << " has not been set.\n";
			verifySuccess = false;
			incorrectLinkCount++;
		}
		else
		{
			correctLinkCount++;
		}
	}
	std::cout << correctLinkCount << " linked successfully.\n";
	std::cout << incorrectLinkCount << " not linked.\n";
	return verifySuccess;
}


void JitCat::expressionEnumerationCallback(const char* name, uintptr_t address)
{
	if (name == std::string("default")
		&& address == 0)
	{
		std::cout << "_jc_enumerate_expressions function symbol not found." << std::endl;
	}
	else
	{
		getPrecompiledExpressionSymbols().insert(std::make_pair(std::string(name), address));
	}
}


void JitCat::globalVariablesEnumerationCallback(const char* name, uintptr_t address)
{
	if (name == std::string("default")
		&& address == 0)
	{
		std::cout << "_jc_enumerate_global_variables function symbol not found." << std::endl;
	}
	else
	{
		getPrecompiledGlobalVariables().insert(std::make_pair(JitCat::defineGlobalVariableName(name), address));
	}
}


void JitCat::linkedFunctionsEnumerationCallback(const char* name, uintptr_t address)
{
	if (name == std::string("default")
		&& address == 0)
	{
		std::cout << "_jc_enumerate_linked_functions function symbol not found." << std::endl;
	}
	else
	{
		getPrecompiledLinkedFunctions().insert(std::make_pair(name, address));
	}
}


void JitCat::stringPoolInitializationCallback(const char* stringValue, uintptr_t address)
{
	if (stringValue == std::string("default")
		&& address == 0)
	{
		std::cout << "_jc_initialize_string_pool function symbol not found." << std::endl;
	}
	else
	{
		Configuration::CatString** stringPoolEntry = reinterpret_cast<Configuration::CatString**>(address);
		*stringPoolEntry = new Configuration::CatString(stringValue);
	}
}


std::unordered_map<std::string, uintptr_t>& JitCat::getPrecompiledExpressionSymbols()
{
	static std::unordered_map<std::string, uintptr_t> precompiledExpressionSymbols;
	return precompiledExpressionSymbols;
}


std::unordered_map<std::string_view, uintptr_t>& JitCat::getPrecompiledGlobalVariables()
{
	static std::unordered_map<std::string_view, uintptr_t> precompiledGlobalVariables;
	return precompiledGlobalVariables;
}


std::unordered_map<std::string, uintptr_t>& JitCat::getPrecompiledLinkedFunctions()
{
	static std::unordered_map<std::string, uintptr_t> precompiledLinkedFunctions;
	return precompiledLinkedFunctions;
}


std::unordered_set<std::string>& JitCat::getGlobalNames()
{
	static std::unordered_set<std::string> globalNames;
	return globalNames;
}


JitCat* JitCat::instance = nullptr;