/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatFunctionParameterDefinitions.h"
#include "jitcat/CatIdentifier.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatScopeRoot.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeInstance.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"

#include <cassert>


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
	parametersScopeId(InvalidScopeID),
	memberFunctionInfo(nullptr)
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
	for (int i = 0; i < parameters->getNumParameters(); i++)
	{
		parameterAssignables.emplace_back(new CatIdentifier(parameters->getParameterName(i), parameters->getParameterLexeme(i)));
		parameterAssignables.back()->typeCheck(compileTimeContext, errorManager, this);
	}
	if (compileTimeContext->getCurrentScope() != nullptr && Tools::equalsWhileIgnoringCase(name, "init"))
	{
		//init function is a special case. We should call the auto generated __init function first.
		CatArgumentList* arguments = new CatArgumentList(lexeme);
		CatScopeRoot* scopeRoot = new CatScopeRoot(compileTimeContext->getCurrentScope()->getScopeId(), lexeme);
		CatMemberFunctionCall* callAutoInit = new CatMemberFunctionCall("__init", scopeRoot, arguments, lexeme);
		scopeBlock->insertStatementFront(callAutoInit);
	}
	if (!scopeBlock->typeCheck(compileTimeContext, errorManager, this))
	{
		compileTimeContext->removeScope(parametersScopeId);
		return false;
	}
	compileTimeContext->removeScope(parametersScopeId);
	compileTimeContext->setCurrentFunction(nullptr);
	bool unreachableCodeDetected = false;
	auto returnCheck = scopeBlock->checkControlFlow(compileTimeContext, errorManager, this, unreachableCodeDetected);
	if(!type->getType().isVoidType() &&(!returnCheck.has_value() || !(*returnCheck)))
	{
		const Tokenizer::Lexeme& typeLexeme = type->getLexeme();
		const Tokenizer::Lexeme& paramLexeme = parameters->getLexeme();
		Tokenizer::Lexeme errorLexeme = Tokenizer::Lexeme(typeLexeme.data(), paramLexeme.data() - typeLexeme.data() + paramLexeme.length() - 1);
		errorManager->compiledWithError("Not all control flow paths return a value.", this, compileTimeContext->getContextName(), errorLexeme);
		return false;
	}
	else if (unreachableCodeDetected)
	{
		return false;
	}
	errorManager->compiledWithoutErrors(this);
	CatScope* currentScope = compileTimeContext->getCurrentScope();
	if (currentScope != nullptr)
	{
		memberFunctionInfo = currentScope->getCustomType()->addMemberFunction(name, compileTimeContext->getScopeType(currentScope->getScopeId()), this);
	}
	return true;
}


std::any jitcat::AST::CatFunctionDefinition::executeFunctionWithPack(CatRuntimeContext* runtimeContext, CatScopeID packScopeId)
{
	std::any result = scopeBlock->execute(runtimeContext);
	runtimeContext->setReturning(false);
	runtimeContext->removeScope(packScopeId);
	return result;
}


std::any jitcat::AST::CatFunctionDefinition::executeFunctionWithArguments(CatRuntimeContext* runtimeContext, const std::vector<std::any>& arguments)
{
	assert(parameterAssignables.size() == arguments.size());
	CatScopeID scopeId = InvalidScopeID;
	std::unique_ptr<CustomTypeInstance> parametersInstance;
	if (parameters->getNumParameters() > 0)
	{
		parametersInstance.reset(createCustomTypeInstance());
		//The parameter values should be of the correct type.
		assert(parametersInstance->typeInfo == parameters->getCustomType());

		scopeId = pushScope(runtimeContext, parametersInstance.get());
		//The scopeId should match the scopeId that was obtained during type checking.
		assert(scopeId == parametersScopeId);

		int i = 0;
		for (auto& iter : parameterAssignables)
		{
			AssignableType assignableType = AssignableType::None;
			std::any target = iter->executeAssignable(runtimeContext, assignableType);
			ASTHelper::doAssignment(target, arguments[i], iter->getType(), assignableType);
			i++;
		}
	}
	return executeFunctionWithPack(runtimeContext, scopeId);
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


CatScopeID jitcat::AST::CatFunctionDefinition::pushScope(CatRuntimeContext* runtimeContext, Reflection::CustomTypeInstance* instance)
{
	return runtimeContext->addCustomTypeScope(parameters->getCustomType(), instance);
}
