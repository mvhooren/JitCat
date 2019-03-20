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
}
namespace jitcat::Parser
{
	class SLRParser;
	struct SLRParseResult;
}

#include <memory>


namespace jitcat
{
	class CatRuntimeContext;
	class ExpressionErrorManager;

	class JitCat
	{
		JitCat();
		~JitCat();
	public:
		static JitCat* get();
		Parser::SLRParseResult* parseExpression(Tokenizer::Document* expression, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const;
		Parser::SLRParseResult* parseFull(Tokenizer::Document* expression, CatRuntimeContext* context, ExpressionErrorManager* errorManager, void* errorContext) const;
		
		//This will clean up as much memory as possible, library features will be broken after this is called.
		//The type registry will be cleared.
		//Memory used by native code generation (LLVM) will also be destroyed. 
		static void destroy();

	private:
		static JitCat* instance;

		std::unique_ptr<Tokenizer::CatTokenizer> tokenizer;
		std::unique_ptr<Grammar::CatGrammar> expressionGrammar;
		std::unique_ptr<Grammar::CatGrammar> fullGrammar;
		std::unique_ptr<Parser::SLRParser> expressionParser;
		std::unique_ptr<Parser::SLRParser> fullParser;
	};

} //End namespace jitcat