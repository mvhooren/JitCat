/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatStaticScope.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;
using namespace jitcat::Reflection;


CatStaticScope::CatStaticScope(bool isValid, CatStaticScope* parentScope, const std::string& scopeName, const Tokenizer::Lexeme& scopeNameLexeme, const Tokenizer::Lexeme& lexeme):
	CatASTNode(lexeme),
	isValid(isValid),
	scopeName(scopeName),
	scopeNameLexeme(scopeNameLexeme),
	parentScope(parentScope),
	scopeType(nullptr)
{
}


CatStaticScope::CatStaticScope(const CatStaticScope& other):
	CatASTNode(other),
	isValid(other.isValid),
	scopeName(other.scopeName),
	scopeNameLexeme(other.scopeNameLexeme),
	parentScope(other.parentScope == nullptr ? nullptr : new CatStaticScope(*other.parentScope.get())),
	scopeType(nullptr)
{
}


CatStaticScope::~CatStaticScope()
{
}


CatASTNode* CatStaticScope::copy() const
{
	return new CatStaticScope(*this);
}


void CatStaticScope::print() const
{
	if (parentScope != nullptr)
	{
		parentScope->print();
		CatLog::log("::");
	}
	CatLog::log(scopeName);

}


CatASTNodeType CatStaticScope::getNodeType() const
{
	return CatASTNodeType::StaticScope;
}


bool CatStaticScope::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (parentScope != nullptr && !parentScope->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	scopeType = nullptr;
	if (isValid)
	{
		if (parentScope != nullptr)
		{
			scopeType = parentScope->getScopeType()->getTypeInfo(scopeName);
		}
		else
		{
			CatScopeID scopeId = InvalidScopeID;
			std::string lowerScopeName = Tools::toLowerCase(scopeName);
			scopeType = compiletimeContext->findType(lowerScopeName, scopeId);
			if (scopeType == nullptr)
			{
				scopeType = TypeRegistry::get()->getTypeInfo(lowerScopeName);
			}
		}
		if (scopeType == nullptr)
		{
			errorManager->compiledWithError(std::string("Static scope not found: ") + scopeName, errorContext, compiletimeContext->getContextName(), scopeNameLexeme);
			return false;
		}
		return true;
	}
	else
	{
		errorManager->compiledWithError(std::string("Not a static scope: ") + scopeName, errorContext, compiletimeContext->getContextName(), scopeNameLexeme);
		return false;
	}
	
}


Reflection::TypeInfo* CatStaticScope::getScopeType() const
{
	return scopeType;
}
