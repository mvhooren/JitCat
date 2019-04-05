/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatFunctionParameterDefinitions.h"

#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeInstance.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


jitcat::AST::CatFunctionDefinition::CatFunctionDefinition(CatTypeNode* type, const std::string& name, CatFunctionParameterDefinitions* parameters, CatScopeBlock* scopeBlock, const Tokenizer::Lexeme& lexeme):
	CatDefinition(lexeme),
	type(type),
	name(name),
	parameters(parameters),
	scopeBlock(scopeBlock),
	parametersScopeId(InvalidScopeID)
{
}


jitcat::AST::CatFunctionDefinition::~CatFunctionDefinition()
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
	}
}


void jitcat::AST::CatFunctionDefinition::print() const
{
	type->print();
	CatLog::log(" ", name, "(");
	parameters->print();
	CatLog::log(")");
	scopeBlock->print();
}


CatASTNodeType jitcat::AST::CatFunctionDefinition::getNodeType()
{
	return CatASTNodeType::FunctionDefinition;
}


bool jitcat::AST::CatFunctionDefinition::typeCheck(CatRuntimeContext* compileTimeContext)
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
		errorManagerHandle = nullptr;
	}	
	ExpressionErrorManager* errorManager = compileTimeContext->getErrorManager();
	errorManagerHandle = errorManager;
	if (!type->typeCheck(compileTimeContext, errorManager, this))
	{
		return false;
	}
	if (!parameters->typeCheck(compileTimeContext, errorManager, this))
	{
		return false;
	}
	compileTimeContext->setCurrentFunction(this);
	if (parameters->getNumParameters() > 0)
	{
		parametersScopeId = compileTimeContext->addCustomTypeScope(parameters->getCustomType());
	}
	if (!scopeBlock->typeCheck(compileTimeContext, errorManager, this))
	{
		compileTimeContext->removeScope(parametersScopeId);
		return false;
	}
	compileTimeContext->removeScope(parametersScopeId);
	compileTimeContext->setCurrentFunction(nullptr);
	if (!type->getType().isVoidType())
	{
		bool unreachableCodeDetected = false;
		auto returnCheck = scopeBlock->checkControlFlow(compileTimeContext, errorManager, this, unreachableCodeDetected);
		if(!returnCheck.has_value() || !(*returnCheck))
		{
			errorManager->compiledWithError("Function is missing a return statement.", this, compileTimeContext->getContextName(), getLexeme());
			return false;
		}
		else if (unreachableCodeDetected)
		{
			return false;
		}
	}
	errorManager->compiledWithoutErrors(this);
	return true;
}


std::any jitcat::AST::CatFunctionDefinition::executeFunctionWithPack(CatRuntimeContext* runtimeContext, Reflection::CustomTypeInstance* parameterPack)
{
	CatScopeID scopeId = InvalidScopeID;
	if (parameters->getNumParameters() > 0)
	{
		assert(parameterPack != nullptr);
		scopeId = runtimeContext->addCustomTypeScope(parameters->getCustomType(), parameterPack);
		//The scopeId should match the scopeId that was obtained during type checking.
		assert(scopeId == parametersScopeId);
		//The parameter values should be of the correct type.
		assert(parameterPack->typeInfo == parameters->getCustomType());
	}
	std::any result = scopeBlock->execute(runtimeContext);
	runtimeContext->setReturning(false);
	runtimeContext->removeScope(scopeId);
	return result;
}


jitcat::Reflection::CustomTypeInfo* jitcat::AST::CatFunctionDefinition::getParametersType() const
{
	return parameters->getCustomType();
}


CatTypeNode* jitcat::AST::CatFunctionDefinition::getReturnTypeNode() const
{
	return type.get();
}


int jitcat::AST::CatFunctionDefinition::getNumParameters() const
{
	return parameters->getNumParameters();
}


const std::string& jitcat::AST::CatFunctionDefinition::getParameterName(int index) const
{
	return parameters->getParameterName(index);
}


const CatTypeNode* jitcat::AST::CatFunctionDefinition::getParameterType(int index) const
{
	return parameters->getParameterType(index);
}


const std::string & jitcat::AST::CatFunctionDefinition::getFunctionName() const
{
	return name;
}


Reflection::CustomTypeInstance* jitcat::AST::CatFunctionDefinition::createCustomTypeInstance() const
{
	return parameters->getCustomType()->createInstance();
}
