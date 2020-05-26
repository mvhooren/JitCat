/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ReflectableHandle.h"

#include <memory>
#include <string>
#include <vector>


namespace jitcat
{
	namespace Parser
	{
		struct SLRParseResult;
	}
	namespace Tokenizer
	{
		class Document;
		class ParseToken;
	}
	namespace AST
	{
		class CatSourceFile;
	}
	class CatLib;


	class SourceFile
	{
	public:
		SourceFile(const std::string& fileContents, CatLib* catLib = nullptr);
		~SourceFile();

		void compile(CatLib& catLib);

		void setSource(const std::string& source);

		bool hasErrors() const;
		AST::CatSourceFile* getAST() const;
		const std::vector<std::unique_ptr<Tokenizer::ParseToken>>& getSourceTokens() const;
		const Tokenizer::Document* getSourceText() const;

	private:
		std::unique_ptr<Tokenizer::Document> sourceText;
		std::unique_ptr<Parser::SLRParseResult> parseResult;

		std::vector<std::unique_ptr<Tokenizer::ParseToken>> sourceTokens;

		Reflection::ReflectableHandle errorManagerHandle;
	};
}