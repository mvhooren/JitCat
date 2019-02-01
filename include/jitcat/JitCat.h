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

namespace jitcat
{
	class CatRuntimeContext;

	class JitCat
	{
		JitCat();
		~JitCat();
	public:
		static JitCat* get();
		Parser::SLRParseResult* parse(Tokenizer::Document* expression, CatRuntimeContext* context) const;
		
		static void cleanup();

	private:
		static JitCat* instance;

		Grammar::CatGrammar* grammar;
		Tokenizer::CatTokenizer* tokenizer;
		Parser::SLRParser* parser;
	};

} //End namespace jitcat