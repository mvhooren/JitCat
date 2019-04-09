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
	class CatRuntimeContext;


	class SourceFile
	{
	public:
		SourceFile(const std::string& fileContents, CatRuntimeContext* context = nullptr);
		~SourceFile();

		void compile(CatRuntimeContext* context);

		void setSource(const std::string& source, CatRuntimeContext* context = nullptr);

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