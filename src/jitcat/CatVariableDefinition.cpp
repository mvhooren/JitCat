/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatVariableDefinition.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScope.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/CatLog.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"

using namespace jitcat;
using namespace jitcat::AST;

CatVariableDefinition::CatVariableDefinition(CatTypeNode* typeNode, const std::string& name, const Tokenizer::Lexeme& lexeme, CatTypedExpression* initialization):
	CatDefinition(lexeme),
	type(typeNode),
	name(name),
	visibility(Reflection::MemberVisibility::Public),
	initializationExpression(initialization),
	memberInfo(nullptr)
{
}


CatVariableDefinition::~CatVariableDefinition()
{
}


void CatVariableDefinition::print() const
{
	type->print();
	Tools::CatLog::log(" ", name);
	if (initializationExpression != nullptr)
	{
		Tools::CatLog::log(" = ");
		initializationExpression->print();
	}
}


CatASTNodeType CatVariableDefinition::getNodeType()
{
	return CatASTNodeType::VariableDefinition;
}


bool CatVariableDefinition::typeCheck(CatRuntimeContext* compileTimeContext)
{
	ExpressionErrorManager* errorManager = compileTimeContext->getErrorManager();
	if (!type->typeCheck(compileTimeContext, errorManager, this))
	{
		return false;
	}
	CatScopeID id = InvalidScopeID;
	if (compileTimeContext->findVariable(Tools::toLowerCase(name), id) != nullptr)
	{
		errorManager->compiledWithError(Tools::append("A variable with name \"", name, "\" already exists."), this, compileTimeContext->getContextName(), getLexeme());
		return false;
	}
	if (initializationExpression != nullptr)
	{
		if (initializationExpression->typeCheck(compileTimeContext, errorManager, this))
		{
			CatGenericType initializationType = initializationExpression->getType();
			if (initializationType != type->getType())
			{
				errorManager->compiledWithError(Tools::append("Initialization of variable \"", name, "\" returns the wrong type. Expected a ", type->getTypeName(), " but the initialization expression returns a ", initializationType.toString(), "."), this, compileTimeContext->getContextName(), getLexeme());
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	CatScope* currentScope = compileTimeContext->getCurrentScope();
	if (currentScope != nullptr)
	{
		memberInfo = currentScope->getCustomType()->addMember(name, type->getType().toWritable());
		memberInfo->visibility = visibility;
	}
	return true;
}


const std::string& CatVariableDefinition::getName() const
{
	return name;
}


const CatTypeNode& CatVariableDefinition::getType() const
{
	return *type.get();
}


CatTypedExpression* jitcat::AST::CatVariableDefinition::releaseInitializationExpression()
{
	return initializationExpression.release();
}


Reflection::MemberVisibility jitcat::AST::CatVariableDefinition::getVariableVisibility() const
{
	return visibility;
}


void jitcat::AST::CatVariableDefinition::setVariableVisibility(Reflection::MemberVisibility variableVisibility)
{
	visibility = variableVisibility;
	if (memberInfo != nullptr)
	{
		memberInfo->visibility = visibility;
	}
}
