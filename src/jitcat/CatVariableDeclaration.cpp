/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatVariableDeclaration.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatAssignmentOperator.h"
#include "jitcat/CatConstruct.h"
#include "jitcat/CatIdentifier.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/CatLog.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/TypeOwnershipSemantics.h"
#include "jitcat/Tools.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatVariableDeclaration::CatVariableDeclaration(CatTypeNode* typeNode, const std::string& name, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme, 
											   const Tokenizer::Lexeme& initializationOperatorLexeme, CatTypedExpression* initialization):
	CatStatement(lexeme),
	type(typeNode),
	name(name),
	nameLexeme(nameLexeme),
	memberInfo(nullptr)
{
	std::unique_ptr<CatArgumentList> arguments;
	if (initialization != nullptr)
	{
		arguments = std::make_unique<CatArgumentList>(initialization->getLexeme(), std::vector<CatTypedExpression*>({initialization}));
	}
	else
	{
		arguments = std::make_unique<CatArgumentList>(lexeme, std::vector<CatTypedExpression*>());
	}
	initializationExpression = std::make_unique<CatConstruct>(lexeme, std::make_unique<CatIdentifier>(name, nameLexeme), std::move(arguments));
}


CatVariableDeclaration::CatVariableDeclaration(const CatVariableDeclaration& other):
	CatStatement(other),
	type(static_cast<CatTypeNode*>(other.type->copy())),
	name(other.name),
	nameLexeme(other.nameLexeme),
	initializationExpression(other.initializationExpression != nullptr ? static_cast<CatAssignmentOperator*>(other.initializationExpression->copy()) : nullptr),
	memberInfo(nullptr)
{
}


CatVariableDeclaration::~CatVariableDeclaration()
{
}


CatASTNode* CatVariableDeclaration::copy() const
{
	return new CatVariableDeclaration(*this);
}


void CatVariableDeclaration::print() const
{
	type->print();
	Tools::CatLog::log(" ");
	if (initializationExpression != nullptr)
	{
		initializationExpression->print();
	}
	else
	{
		Tools::CatLog::log(name);
	}
}


CatASTNodeType CatVariableDeclaration::getNodeType() const
{
	return CatASTNodeType::VariableDeclaration;
}


bool CatVariableDeclaration::defineCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!type->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	CatScopeID id = InvalidScopeID;
	if (memberInfo == nullptr && compiletimeContext->findVariable(Tools::toLowerCase(name), id) != nullptr)
	{
		errorManager->compiledWithError(Tools::append("A variable with name \"", name, "\" already exists."), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	CatScope* currentScope = compiletimeContext->getCurrentScope();
	if (currentScope != nullptr && memberInfo == nullptr)
	{
		memberInfo = currentScope->getCustomType()->addMember(name, type->getType().toWritable());
	}
	return true;
}


bool CatVariableDeclaration::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!type->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	CatScopeID id = InvalidScopeID;
	if (memberInfo == nullptr)
	{
		if (compiletimeContext->findVariable(Tools::toLowerCase(name), id) != nullptr)
		{
			errorManager->compiledWithError(Tools::append("A variable with name \"", name, "\" already exists."), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}

	CatScope* currentScope = compiletimeContext->getCurrentScope();
	if (currentScope != nullptr && memberInfo == nullptr)
	{
		memberInfo = currentScope->getCustomType()->addMember(name, type->getType().toWritable());
	}

	return initializationExpression->typeCheck(compiletimeContext, errorManager, errorContext);
}


CatStatement* CatVariableDeclaration::constCollapse(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (initializationExpression != nullptr)
	{
		ASTHelper::updatePointerIfChanged(initializationExpression,	initializationExpression->constCollapse(compiletimeContext, errorManager, errorContext));
	}
	return this;
}


std::any CatVariableDeclaration::execute(CatRuntimeContext* runtimeContext)
{
	return initializationExpression->execute(runtimeContext);
}


const std::string& CatVariableDeclaration::getName() const
{
	return name;
}


const CatTypeNode& CatVariableDeclaration::getType() const
{
	return *type.get();
}


const CatStatement* CatVariableDeclaration::getInitializationExpression() const
{
	return initializationExpression.get();
}
