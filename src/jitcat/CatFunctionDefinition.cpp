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
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


jitcat::AST::CatFunctionDefinition::CatFunctionDefinition(CatTypeNode* type, const std::string& name, CatFunctionParameterDefinitions* parameters, CatScopeBlock* scopeBlock):
	type(type),
	name(name),
	parameters(parameters),
	scopeBlock(scopeBlock)
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
	CatScopeID scopeId = compileTimeContext->addCustomTypeScope(parameters->getCustomType());
	if (!scopeBlock->typeCheck(compileTimeContext, errorManager, this))
	{
		return false;
	}
	compileTimeContext->removeScope(scopeId);
	errorManager->compiledWithoutErrors(this);
	return true;
}
