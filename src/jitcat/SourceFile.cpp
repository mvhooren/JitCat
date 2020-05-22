/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/SourceFile.h"
#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatSourceFile.h"
#include "jitcat/Document.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/JitCat.h"
#include "jitcat/ParseToken.h"
#include "jitcat/Reflectable.h"
#include "jitcat/SLRParseResult.h"


using namespace jitcat;
using namespace jitcat::Parser;
using namespace jitcat::Tokenizer;


SourceFile::SourceFile(const std::string& fileContents, CatRuntimeContext* context)
{
	setSource(fileContents, context);
}


SourceFile::~SourceFile()
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
	}
}


void SourceFile::compile(CatRuntimeContext* context)
{
	if (context == nullptr)
	{
		context = &CatRuntimeContext::defaultContext;
		context->getErrorManager()->clear();
	}

	ExpressionErrorManager* errorManager = nullptr;
	errorManager = context->getErrorManager();
	errorManagerHandle = errorManager;
	errorManager->setCurrentDocument(sourceText.get());
	sourceTokens.clear();
	parseResult.reset(JitCat::get()->parseFull(sourceText.get(), sourceTokens, context, errorManager, this));
	if (parseResult->success)
	{
		AST::CatSourceFile* sourceFileNode = parseResult->getNode<AST::CatSourceFile>();
		if (sourceFileNode->typeCheck(context, errorManager, this))
		{
			errorManager->compiledWithoutErrors(this);
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
	errorManager->setCurrentDocument(nullptr);
}


void jitcat::SourceFile::setSource(const std::string& source, CatRuntimeContext* context)
{
	if (context == nullptr)
	{
		context = &CatRuntimeContext::defaultContext;
		context->getErrorManager()->clear();
	}
	context->getErrorManager()->errorSourceDeleted(this);	
	sourceText = std::make_unique<Document>(source.c_str(), source.size());
	compile(context);
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
