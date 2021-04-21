/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Grammar
{
	class CatGrammar;
}
namespace jitcat::Tokenizer
{
	class CatTokenizer;
	class Document;
	class ParseToken;
}
namespace jitcat::Parser
{
	class SLRParser;
	struct SLRParseResult;
}

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace jitcat
{
	class CatRuntimeContext;
	class ExpressionErrorManager;
	class PrecompilationContext;

	class JitCat
	{
		JitCat();
		~JitCat();
	public:
		static JitCat* get();
		std::unique_ptr<Parser::SLRParseResult> parseExpression(Tokenizer::Document* expression, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const;
		std::unique_ptr<Parser::SLRParseResult> parseStatement(Tokenizer::Document* statement, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const;
		std::unique_ptr<Parser::SLRParseResult> parseFull(Tokenizer::Document* expression, std::vector<std::unique_ptr<Tokenizer::ParseToken>>& tokens, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const;
		std::unique_ptr<Parser::SLRParseResult> parseFull(Tokenizer::Document* expression, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const;
		std::shared_ptr<PrecompilationContext> createPrecompilationContext() const;

		static uintptr_t getPrecompiledSymbol(const std::string& name);

		bool setPrecompiledGlobalVariable(const std::string_view variableName, unsigned char* value);
		bool setPrecompiledLinkedFunction(const std::string mangledFunctionName, uintptr_t address);

		//This will clean up as much memory as possible, library features will be broken after this is called.
		//The type registry will be cleared.
		//Memory used by native code generation (LLVM) will also be destroyed. 
		static void destroy();

		static std::string_view defineGlobalVariableName(const std::string& globalName);

		//Checks whether all the globals scopes and linked functions have been set to a non-null value.
		//null values will be printed to std::out.
		bool verifyLinkage();

	private:
		static void expressionEnumerationCallback(const char* name, uintptr_t address);
		static void globalVariablesEnumerationCallback(const char* name, uintptr_t address);
		static void linkedFunctionsEnumerationCallback(const char* name, uintptr_t address);
	private:
		static JitCat* instance;
		static std::unordered_map<std::string, uintptr_t>* precompiledExpressionSymbols;
		static std::unordered_map<std::string_view, uintptr_t>* precompiledGlobalVariables;
		static std::unordered_map<std::string, uintptr_t>* precompiledLinkedFunctions;
		static std::unordered_set<std::string>* globalNames;

		std::unique_ptr<Tokenizer::CatTokenizer> tokenizer;
		
		std::unique_ptr<Grammar::CatGrammar> expressionGrammar;
		std::unique_ptr<Grammar::CatGrammar> statementGrammar;
		std::unique_ptr<Grammar::CatGrammar> fullGrammar;

		std::unique_ptr<Parser::SLRParser> expressionParser;
		std::unique_ptr<Parser::SLRParser> statementParser;
		std::unique_ptr<Parser::SLRParser> fullParser;

	};

} //End namespace jitcat