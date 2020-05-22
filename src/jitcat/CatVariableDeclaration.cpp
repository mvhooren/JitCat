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
#include "jitcat/TypeOwnershipSemantics.h"
#include "jitcat/Tools.h"

using namespace jitcat;
using namespace jitcat::AST;


CatVariableDeclaration::CatVariableDeclaration(CatTypeNode* typeNode, const std::string& name, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme, 
											   const Tokenizer::Lexeme& initializationOperatorLexeme, CatTypedExpression* initialization):
	CatStatement(lexeme),
	type(typeNode),
	name(name),
	nameLexeme(nameLexeme),
	memberInfo(nullptr)
{
	if (initialization != nullptr)
	{
		initializationExpression = std::make_unique<CatAssignmentOperator>(new CatIdentifier(name, nameLexeme), initialization, lexeme, initializationOperatorLexeme);
	}
}


jitcat::AST::CatVariableDeclaration::CatVariableDeclaration(const CatVariableDeclaration& other):
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


CatASTNode* jitcat::AST::CatVariableDeclaration::copy() const
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
		Reflection::TypeOwnershipSemantics initExpressionOwnership = Reflection::TypeOwnershipSemantics::Value;
		if (type->getType().isPointerToReflectableObjectType() || type->getType().isReflectableHandleType())
		{
			if (type->getType().getOwnershipSemantics() != Reflection::TypeOwnershipSemantics::Owned
				&& type->getType().getOwnershipSemantics() != Reflection::TypeOwnershipSemantics::Shared)
			{
				initExpressionOwnership = Reflection::TypeOwnershipSemantics::Weak;
			}
		}
		CatGenericType initExpressionType = type->getType();
		initExpressionType.setOwnershipSemantics(initExpressionOwnership);
		initializationExpression = std::make_unique<CatAssignmentOperator>(new CatIdentifier(name, nameLexeme), new CatLiteral(type->getType().createDefault(), initExpressionType, nameLexeme), nameLexeme, nameLexeme);
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
