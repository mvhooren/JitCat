#pragma once

#include "jitcat/ReflectableHandle.h"

#include <memory>
#include <string>


namespace jitcat
{
	namespace Parser
	{
		struct SLRParseResult;
	}
	namespace Tokenizer
	{
		class Document;
	}
	class CatRuntimeContext;


	class SourceFile
	{
	public:
		SourceFile(const std::string& fileContents, CatRuntimeContext* context = nullptr);
		~SourceFile();

		void compile(CatRuntimeContext* context);

	private:
		std::unique_ptr<Tokenizer::Document> sourceText;
		std::unique_ptr<Parser::SLRParseResult> parseResult;

		Reflection::ReflectableHandle errorManagerHandle;
	};
}