/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/SourceFile.h"
#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatLib.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatSourceFile.h"
#include "jitcat/Document.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/JitCat.h"
#include "jitcat/ParseToken.h"
#include "jitcat/Reflectable.h"
#include "jitcat/SLRParseResult.h"
#include "jitcat/TypeRegistry.h"


using namespace jitcat;
using namespace jitcat::Parser;
using namespace jitcat::Reflection;
using namespace jitcat::Tokenizer;


SourceFile::SourceFile(const std::string& fileContents, CatLib* catLib)
{
	setSource(fileContents);
	if (catLib != nullptr)
	{
		compile(*catLib);
	}
}


SourceFile::~SourceFile()
{
	if (errorManagerHandle.getIsValid())
	{
		reinterpret_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
	}
}


void SourceFile::compile(CatLib& catLib)
{
	ExpressionErrorManager& errorManager = catLib.getErrorManager();
	errorManagerHandle.setReflectable(reinterpret_cast<unsigned char*>(&errorManager), TypeRegistry::get()->registerType<ExpressionErrorManager>());

	errorManager.setCurrentDocument(sourceText.get());
	sourceTokens.clear();
	parseResult = JitCat::get()->parseFull(sourceText.get(), sourceTokens, catLib.getRuntimeContext(), &errorManager, this);
	if (parseResult->success)
	{
		AST::CatSourceFile* sourceFileNode = parseResult->getNode<AST::CatSourceFile>();
		if (sourceFileNode->compile(catLib))
		{
			errorManager.compiledWithoutErrors(this);
		}
		else
		{
			parseResult->success = false;
		}
	}
	else
	{
		parseResult->success = false;
	}
	errorManager.setCurrentDocument(nullptr);
}


void jitcat::SourceFile::setSource(const std::string& source)
{
	sourceText = std::make_unique<Document>(source.c_str(), source.size());
}


bool jitcat::SourceFile::hasErrors() const
{
	return parseResult == nullptr || !parseResult->success;
}


AST::CatSourceFile* jitcat::SourceFile::getAST() const
{
	if (parseResult != nullptr && parseResult->success)
	{
		return parseResult->getNode<AST::CatSourceFile>();
	}
	return nullptr;
}


const std::vector<std::unique_ptr<Tokenizer::ParseToken>>& jitcat::SourceFile::getSourceTokens() const
{
	return sourceTokens;
}


const Tokenizer::Document* jitcat::SourceFile::getSourceText() const
{
	return sourceText.get();
}
