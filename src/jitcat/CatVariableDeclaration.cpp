/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatVariableDeclaration.h"
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


CatVariableDeclaration::CatVariableDeclaration(CatTypeNode* typeNode, const std::string& name, const Tokenizer::Lexeme& lexeme, CatTypedExpression* initialization):
	CatStatement(lexeme),
	type(typeNode),
	name(name),
	initializationExpression(initialization),
	memberInfo(nullptr)
{
}


CatVariableDeclaration::~CatVariableDeclaration()
{
}


void CatVariableDeclaration::print() const
{
	type->print();
	Tools::CatLog::log(" ", name);
	if (initializationExpression != nullptr)
	{
		Tools::CatLog::log(" = ");
		initializationExpression->print();
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
	if (initializationExpression != nullptr)
	{
		if (initializationExpression->typeCheck(compiletimeContext, errorManager, errorContext))
		{
			CatGenericType initializationType = initializationExpression->getType();
			if (initializationType != type->getType())
			{
				errorManager->compiledWithError(Tools::append("Initialization of variable \"", name, "\" returns the wrong type. Expected a ", type->getTypeName(), " but the initialization expression returns a ", initializationType.toString(), "."), errorContext, compiletimeContext->getContextName(), getLexeme());
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	CatScope* currentScope = compiletimeContext->getCurrentScope();
	if (currentScope != nullptr)
	{
		memberInfo = currentScope->getCustomType()->addMember(name, type->getType().toWritable());
	}
	return true;
}


std::any jitcat::AST::CatVariableDeclaration::execute(CatRuntimeContext* runtimeContext)
{
	if (memberInfo != nullptr)
	{
		Reflection::AssignableType assignableType = Reflection::AssignableType::None;
		std::any assignableRef = memberInfo->getAssignableMemberReference(runtimeContext->getCurrentScopeObject(), assignableType);
		std::any source;
		if (initializationExpression != nullptr)
		{
			source = initializationExpression->execute(runtimeContext);
		}
		else
		{
			source = type->getType().createDefault();
		}
		ASTHelper::doAssignment(assignableRef, source, type->getType(), assignableType);
	}
	return std::any();
}


const std::string& jitcat::AST::CatVariableDeclaration::getName() const
{
	return name;
}


const CatTypeNode& jitcat::AST::CatVariableDeclaration::getType() const
{
	return *type.get();
}
