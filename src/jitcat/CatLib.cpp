/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatLib.h"
#include "jitcat/CatSourceFile.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ErrorContext.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/JitCat.h"
#include "jitcat/Document.h"
#include "jitcat/SLRParseResult.h"
#include "jitcat/Tools.h"

#include <cassert>
#include <iostream>

using namespace jitcat;
using namespace AST;
using namespace Reflection;

CatLib::CatLib(const std::string& libName, std::shared_ptr<PrecompilationContext> precompilationContext, std::function<void(const std::string&, int, int, int)> errorHandler):
	name(libName),
	errorManager(std::make_unique<ExpressionErrorManager>(errorHandler)),
	context(std::make_unique<CatRuntimeContext>(libName, errorManager.get()))
{
	context->setPrecompilationContext(precompilationContext);
}


CatLib::~CatLib()
{
}


CatScopeID CatLib::addStaticScope(Reflection::TypeInfo* typeInfo, unsigned char* scopeObject, const std::string& scopeName)
{
	return context->addStaticScope(typeInfo, scopeObject, scopeName);
}


void CatLib::removeStaticScope(CatScopeID id)
{
	context->removeScope(id);
}


void CatLib::replaceStaticScopeObject(CatScopeID id, unsigned char* scopeObject)
{
	context->setScopeObject(id, scopeObject);
}


bool CatLib::addSource(const std::string& translationUnitName, Tokenizer::Document& translationUnitCode)
{
	ErrorContext errorContext(context.get(), name);
	errorManager->setCurrentDocument(&translationUnitCode);
	std::unique_ptr<Parser::SLRParseResult> result = JitCat::get()->parseFull(&translationUnitCode, context.get(), errorManager.get(), this);
	if (result->success)

	{
		CatSourceFile* sourceFile = result->releaseNode<CatSourceFile>();
		sourceFiles.emplace_back(sourceFile);
		if (sourceFile->compile(*this))
		{
			errorManager->setCurrentDocument(nullptr);
			return true;
		}
	}
	errorManager->setCurrentDocument(nullptr);
	return false;
}


const std::string& CatLib::getName() const
{
	return name;
}


Reflection::TypeInfo* CatLib::getTypeInfo(const std::string& typeName) const
{
	//Check each source file for a class definition of the given typeName.
	//This should be made more efficient in the future by storing every type in a map in the CatLib.
	for (auto& iter : sourceFiles)
	{
		CustomTypeInfo* customType = iter->getCustomType();
		TypeInfo* typeInfo = customType->getTypeInfo(typeName);
		if (typeInfo != nullptr)
		{
			return typeInfo;
		}
	}
	return nullptr;
}


ExpressionErrorManager& CatLib::getErrorManager() const
{
	return *errorManager.get();
}


CatRuntimeContext* CatLib::getRuntimeContext() const
{
	return context.get();
}
