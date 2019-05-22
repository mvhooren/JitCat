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
#include "jitcat/CustomTypeMemberFunctionInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"

#include <cassert>


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


jitcat::AST::CatFunctionDefinition::CatFunctionDefinition(CatTypeNode* type, const std::string& name, const Tokenizer::Lexeme& nameLexeme, CatFunctionParameterDefinitions* parameters, CatScopeBlock* scopeBlock, const Tokenizer::Lexeme& lexeme):
	CatDefinition(lexeme),
	type(type),
	name(name),
	visibility(MemberVisibility::Public),
	nameLexeme(nameLexeme),
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
	if (parameters->getNumParameters() > 0)
	{
		parametersScopeId = compileTimeContext->addCustomTypeScope(parameters->getCustomType());
	}
	CatScope* previousScope = compileTimeContext->getCurrentScope();
	compileTimeContext->setCurrentScope(this);
	if (!parameters->typeCheck(compileTimeContext, errorManager, this))
	{
		return false;
	}
	compileTimeContext->setCurrentFunction(this);
	for (int i = 0; i < parameters->getNumParameters(); i++)
	{
		parameterAssignables.emplace_back(new CatIdentifier(parameters->getParameterName(i), parameters->getParameterLexeme(i)));
		parameterAssignables.back()->typeCheck(compileTimeContext, errorManager, this);
	}
	if (compileTimeContext->getCurrentScope() != nullptr && Tools::equalsWhileIgnoringCase(name, "init"))
	{
		visibility = MemberVisibility::Constructor;
		//init function is a special case. We should call the auto generated __init function first.
		CatArgumentList* arguments = new CatArgumentList(lexeme);
		CatScopeRoot* scopeRoot = new CatScopeRoot(compileTimeContext->getCurrentScope()->getScopeId(), lexeme);
		CatMemberFunctionCall* callAutoInit = new CatMemberFunctionCall("__init", nameLexeme, scopeRoot, arguments, lexeme);
		scopeBlock->insertStatementFront(callAutoInit);
	}
	if (!scopeBlock->typeCheck(compileTimeContext, errorManager, this))
	{
		compileTimeContext->removeScope(parametersScopeId);
		compileTimeContext->setCurrentScope(previousScope);
		return false;
	}
	compileTimeContext->removeScope(parametersScopeId);
	compileTimeContext->setCurrentScope(previousScope);
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
		CatScopeID existingFunctionScopeId = InvalidScopeID;
		if (compileTimeContext->findFunction(Tools::toLowerCase(name), existingFunctionScopeId) != nullptr && existingFunctionScopeId == currentScope->getScopeId())
		{
			errorManager->compiledWithError(Tools::append("A function with name \"", name, "\" already exists."), this, compileTimeContext->getContextName(), nameLexeme);
			return false;
		}
		memberFunctionInfo = currentScope->getCustomType()->addMemberFunction(name, CatGenericType(compileTimeContext->getScopeType(currentScope->getScopeId()), type->getType().getOwnershipSemantics()), this);
		memberFunctionInfo->visibility = visibility;
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
			ASTHelper::doAssignment(target, arguments[i], iter->getType(), iter->getType(), assignableType, AssignableType::None);
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


Reflection::MemberVisibility jitcat::AST::CatFunctionDefinition::getFunctionVisibility() const
{
	return visibility;
}


void jitcat::AST::CatFunctionDefinition::setFunctionVisibility(Reflection::MemberVisibility functionVisibility)
{
	visibility = functionVisibility;
	if (memberFunctionInfo != nullptr)
	{
		memberFunctionInfo->visibility = visibility;
	}
}


const std::string & jitcat::AST::CatFunctionDefinition::getFunctionName() const
{
	return name;
}


CatScopeID CatFunctionDefinition::getScopeId() const
{
	return parametersScopeId;
}


Reflection::CustomTypeInfo* CatFunctionDefinition::getCustomType()
{
	return parameters->getCustomType();
}


Reflection::CustomTypeInstance* jitcat::AST::CatFunctionDefinition::createCustomTypeInstance() const
{
	return parameters->getCustomType()->createInstance();
}


CatScopeID jitcat::AST::CatFunctionDefinition::pushScope(CatRuntimeContext* runtimeContext, Reflection::CustomTypeInstance* instance)
{
	return runtimeContext->addCustomTypeScope(parameters->getCustomType(), instance);
}
