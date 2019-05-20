/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatVariableDeclaration.h"
#include "jitcat/CatAssignmentOperator.h"
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
#include "jitcat/Tools.h"

using namespace jitcat;
using namespace jitcat::AST;


CatVariableDeclaration::CatVariableDeclaration(CatTypeNode* typeNode, const std::string& name, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme, CatTypedExpression* initialization):
	CatStatement(lexeme),
	type(typeNode),
	name(name),
	nameLexeme(nameLexeme),
	memberInfo(nullptr)
{
	if (initialization != nullptr)
	{
		initializationExpression.reset(new CatAssignmentOperator(new CatIdentifier(name, nameLexeme), initialization, lexeme));
	}
}


CatVariableDeclaration::~CatVariableDeclaration()
{
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


CatASTNodeType CatVariableDeclaration::getNodeType()
{
	return CatASTNodeType::VariableDeclaration;
}


bool jitcat::AST::CatVariableDeclaration::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (!type->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}
	CatScopeID id = InvalidScopeID;
	if (compiletimeContext->findVariable(Tools::toLowerCase(name), id) != nullptr)
	{
		errorManager->compiledWithError(Tools::append("A variable with name \"", name, "\" already exists."), errorContext, compiletimeContext->getContextName(), getLexeme());
		return false;
	}
	if (initializationExpression == nullptr)
	{
		initializationExpression.reset(new CatAssignmentOperator(new CatIdentifier(name, nameLexeme), new CatLiteral(type->getType().createDefault(), type->getType().toValueOwnership(), nameLexeme), nameLexeme));
	}

	CatScope* currentScope = compiletimeContext->getCurrentScope();
	if (currentScope != nullptr)
	{
		memberInfo = currentScope->getCustomType()->addMember(name, type->getType().toWritable());
	}

	if (!initializationExpression->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		return false;
	}

	return true;
}


std::any jitcat::AST::CatVariableDeclaration::execute(CatRuntimeContext* runtimeContext)
{
	return initializationExpression->execute(runtimeContext);
}


const std::string& jitcat::AST::CatVariableDeclaration::getName() const
{
	return name;
}


const CatTypeNode& jitcat::AST::CatVariableDeclaration::getType() const
{
	return *type.get();
}
