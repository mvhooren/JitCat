/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatScopeBlock.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatVariableDeclaration.h"
#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/ObjectInstance.h"

#include <iostream>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatScopeBlock::CatScopeBlock(const std::vector<CatStatement*>& statementList, const Tokenizer::Lexeme& lexeme):
	CatStatement(lexeme),
	customType(makeTypeInfo<CustomTypeInfo>("__ScopeLocals")),
	scopeId(InvalidScopeID)
{
	for (auto& iter : statementList)
	{
		statements.emplace_back(iter);
	}
}


jitcat::AST::CatScopeBlock::CatScopeBlock(const CatScopeBlock& other):
	CatStatement(other),
	customType(makeTypeInfo<CustomTypeInfo>("__ScopeLocals")),
	scopeId(InvalidScopeID)
{
	for (auto& iter : other.statements)
	{
		statements.emplace_back(static_cast<CatStatement*>(iter->copy()));
	}
}


CatScopeBlock::~CatScopeBlock()
{
}


CatASTNode* jitcat::AST::CatScopeBlock::copy() const
{
	return new CatScopeBlock(*this);
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


CatASTNodeType CatScopeBlock::getNodeType() const
{
	return CatASTNodeType::ScopeBlock;
}


bool jitcat::AST::CatScopeBlock::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	CatScopeID myScopeId = compiletimeContext->addScope(customType.get(), nullptr, false);
	CatScope* previousScope = compiletimeContext->getCurrentScope();
	compiletimeContext->setCurrentScope(this);
	bool noErrors = true;
	for (auto& iter : statements)
	{
		noErrors &= iter->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	if (noErrors)
	{
		for (auto& iter : statements)
		{
			noErrors &= iter->typeCheck(compiletimeContext, errorManager, errorContext);
		}
	}
	compiletimeContext->removeScope(myScopeId);
	compiletimeContext->setCurrentScope(previousScope);
	return noErrors;
}


CatStatement* CatScopeBlock::constCollapse(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	for (auto& iter : statements)
	{
		ASTHelper::updatePointerIfChanged(iter, iter->constCollapse(compiletimeContext, errorManager, errorContext));
	}
	return this;
}


std::any jitcat::AST::CatScopeBlock::execute(CatRuntimeContext* runtimeContext)
{
	unsigned char* scopeMem = static_cast<unsigned char*>(alloca(customType->getTypeSize()));
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		if (customType->getTypeSize() > 0)
		{
			std::cout << "(CatScopeBlock::execute) Stack-allocated buffer of size " << std::dec << customType->getTypeSize() << ": " << std::hex << reinterpret_cast<uintptr_t>(scopeMem) << "\n";
		}
	}
	customType->placementConstruct(scopeMem, customType->getTypeSize());
	scopeId = runtimeContext->addScope(customType.get(), scopeMem, false);
	CatScope* previousScope = runtimeContext->getCurrentScope();
	runtimeContext->setCurrentScope(this);
	std::any result = std::any();
	for (auto& iter : statements)
	{
		if (iter->getNodeType() == CatASTNodeType::ReturnStatement)
		{
			runtimeContext->setReturning(true);
		}
		result = iter->execute(runtimeContext);
		if (runtimeContext->getIsReturning())
		{
			break;
		}
	}
	runtimeContext->removeScope(scopeId);
	runtimeContext->setCurrentScope(previousScope);
	customType->placementDestruct(scopeMem, customType->getTypeSize());
	return result;
}


std::optional<bool> jitcat::AST::CatScopeBlock::checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected) const
{
	bool controlFlowReturns = false;
	for (auto& iter : statements)
	{
		auto returns = iter->checkControlFlow(compiletimeContext, errorManager, errorContext, unreachableCodeDetected);
		if (!controlFlowReturns && returns.has_value() && (*returns))
		{
			controlFlowReturns = true;		
		}
		else if (controlFlowReturns)
		{
			unreachableCodeDetected = true;
			errorManager->compiledWithError("Code is unreachable.", errorContext, compiletimeContext->getContextName(), iter->getLexeme());
			return true;
		}
	}
	return controlFlowReturns;
}


bool jitcat::AST::CatScopeBlock::containsReturnStatement() const
{
	for (auto& iter : statements)
	{
		if (iter->getNodeType() == CatASTNodeType::ReturnStatement)
		{
			return true;
		}
	}
	return false;
}


Reflection::CustomTypeInfo* jitcat::AST::CatScopeBlock::getCustomType()
{
	return customType.get();
}


CatScopeID jitcat::AST::CatScopeBlock::getScopeId() const
{
	return scopeId;
}


const std::vector<std::unique_ptr<CatStatement>>& jitcat::AST::CatScopeBlock::getStatements() const
{
	return statements;
}


void jitcat::AST::CatScopeBlock::insertStatementFront(CatStatement* statement)
{
	statements.emplace(statements.begin(), statement);
}