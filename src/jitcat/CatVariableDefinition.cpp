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
#include "jitcat/TypeRegistry.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;


CatVariableDefinition::CatVariableDefinition(CatTypeNode* typeNode, const std::string& name, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& initializationOperatorLexeme, CatTypedExpression* initialization):
	CatDefinition(lexeme),
	type(typeNode),
	name(name),
	visibility(Reflection::MemberVisibility::Public),
	initializationExpression(initialization),
	initOperatorLexeme(initializationOperatorLexeme),
	memberInfo(nullptr)
{
}


CatVariableDefinition::CatVariableDefinition(const CatVariableDefinition& other):
	CatDefinition(other),
	type(static_cast<CatTypeNode*>(other.type->copy())),
	name(other.name),
	visibility(other.visibility),
	initializationExpression(other.initializationExpression != nullptr ? static_cast<CatTypedExpression*>(other.initializationExpression->copy()) : nullptr),
	memberInfo(nullptr)
{
}


CatVariableDefinition::~CatVariableDefinition()
{
	if (errorManagerHandle.getIsValid())
	{
		reinterpret_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
	}
}


CatASTNode* CatVariableDefinition::copy() const
{
	return new CatVariableDefinition(*this);
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


CatASTNodeType CatVariableDefinition::getNodeType() const
{
	return CatASTNodeType::VariableDefinition;
}


bool CatVariableDefinition::typeGatheringCheck(CatRuntimeContext* compileTimeContext)
{
	return true;
}


bool CatVariableDefinition::defineCheck(CatRuntimeContext* compileTimeContext, std::vector<const CatASTNode*>& loopDetectionStack)
{
	if (errorManagerHandle.getIsValid())
	{
		reinterpret_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
		errorManagerHandle = nullptr;
	}	
	ExpressionErrorManager* errorManager = compileTimeContext->getErrorManager();
	errorManagerHandle.setReflectable(reinterpret_cast<unsigned char*>(errorManager), TypeRegistry::get()->registerType<ExpressionErrorManager>());

	loopDetectionStack.push_back(this);
	bool result = type->defineCheck(compileTimeContext, errorManager, this, loopDetectionStack);
	loopDetectionStack.pop_back();
	if (!result)
	{
		return false;
	}
	if (!type->typeCheck(compileTimeContext, errorManager, this))
	{
		return false;
	}

	CatScopeID id = InvalidScopeID;
	if (memberInfo == nullptr && compileTimeContext->findVariable(Tools::toLowerCase(name), id) != nullptr)
	{
		errorManager->compiledWithError(Tools::append("A variable with name \"", name, "\" already exists."), this, compileTimeContext->getContextName(), getLexeme());
		return false;
	}
	CatScope* currentScope = compileTimeContext->getCurrentScope();
	if (memberInfo == nullptr && currentScope != nullptr)
	{
		memberInfo = currentScope->getCustomType()->addMember(name, type->getType().toWritable());
		memberInfo->setVisibility(visibility);
	}
	return true;
}


bool CatVariableDefinition::typeCheck(CatRuntimeContext* compileTimeContext)
{
	if (errorManagerHandle.getIsValid())
	{
		reinterpret_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
		errorManagerHandle = nullptr;
	}	
	ExpressionErrorManager* errorManager = compileTimeContext->getErrorManager();
	errorManagerHandle.setReflectable(reinterpret_cast<unsigned char*>(errorManager), TypeRegistry::get()->registerType<ExpressionErrorManager>());

	if (!type->typeCheck(compileTimeContext, errorManager, this))
	{
		return false;
	}
	CatScopeID id = InvalidScopeID;
	if (memberInfo == nullptr && compileTimeContext->findVariable(Tools::toLowerCase(name), id) != nullptr)
	{
		errorManager->compiledWithError(Tools::append("A variable with name \"", name, "\" already exists."), this, compileTimeContext->getContextName(), getLexeme());
		return false;
	}
	if (initializationExpression != nullptr)
	{
		if (initializationExpression->typeCheck(compileTimeContext, errorManager, this))
		{
			CatGenericType initializationType = initializationExpression->getType();
			if (!initializationType.compare(type->getType(), false, false))
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
	if (memberInfo == nullptr && currentScope != nullptr)
	{
		memberInfo = currentScope->getCustomType()->addMember(name, type->getType().toWritable());
		memberInfo->setVisibility(visibility);
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


const CatTypedExpression* CatVariableDefinition::getInitializationExpression() const
{
	return initializationExpression.get();
}


Tokenizer::Lexeme CatVariableDefinition::getInitializationOperatorLexeme() const
{
	return initOperatorLexeme;
}


Reflection::MemberVisibility CatVariableDefinition::getVariableVisibility() const
{
	return visibility;
}


void CatVariableDefinition::setVariableVisibility(Reflection::MemberVisibility variableVisibility)
{
	visibility = variableVisibility;
	if (memberInfo != nullptr)
	{
		memberInfo->setVisibility(visibility);
	}
}
