/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeInstance.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatScopeBlock::CatScopeBlock(const std::vector<CatStatement*>& statementList, const Tokenizer::Lexeme& lexeme):
	CatStatement(lexeme),
	customType(new CustomTypeInfo(nullptr))
{
	for (auto& iter : statementList)
	{
		statements.emplace_back(iter);
	}
}


CatScopeBlock::~CatScopeBlock()
{
}


void CatScopeBlock::print() const
{
	Tools::CatLog::log("{\n");
	for (auto& iter : statements)
	{
		iter->print();
		Tools::CatLog::log("\n");
	}
	Tools::CatLog::log("}\n");
}


CatASTNodeType CatScopeBlock::getNodeType()
{
	return CatASTNodeType::ScopeBlock;
}


bool jitcat::AST::CatScopeBlock::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	CatScopeID myScopeId = compiletimeContext->addCustomTypeScope(customType.get());
	bool anyErrors = false;
	for (auto& iter : statements)
	{
		anyErrors |= iter->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	compiletimeContext->removeScope(myScopeId);
	return !anyErrors;
}


std::any jitcat::AST::CatScopeBlock::execute(CatRuntimeContext* runtimeContext)
{
	std::unique_ptr<CustomTypeInstance> scopeInstance;
	if (customType != nullptr)
	{
		scopeInstance.reset(customType->createInstance());
		runtimeContext->addCustomTypeScope(customType.get(), scopeInstance.get());
	}
	for (auto& iter : statements)
	{
		if (iter->getNodeType() == CatASTNodeType::ReturnStatement)
		{
			if (scopeInstance != nullptr)
			{
				customType->instanceDestructor(scopeInstance.get());
			}
			return iter->execute(runtimeContext);
		}
		else
		{
			iter->execute(runtimeContext);
		}
	}

	if (scopeInstance != nullptr)
	{
		customType->instanceDestructor(scopeInstance.get());
	}
	return std::any();
}
