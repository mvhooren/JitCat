/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatFunctionParameterDefinitions.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatIdentifier.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatScopeRoot.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeMemberFunctionInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/ObjectInstance.h"
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
	lowerCaseName(Tools::toLowerCase(name)),
	visibility(MemberVisibility::Public),
	nameLexeme(nameLexeme),
	parameters(parameters),
	scopeBlock(scopeBlock),
	parametersScopeId(InvalidScopeID),
	memberFunctionInfo(nullptr)
{
}


jitcat::AST::CatFunctionDefinition::CatFunctionDefinition(const CatFunctionDefinition& other):
	CatDefinition(other),
	type(static_cast<CatTypeNode*>(other.type->copy())),
	name(other.name),
	lowerCaseName(other.lowerCaseName),
	visibility(other.visibility),
	nameLexeme(other.nameLexeme),
	parameters(static_cast<CatFunctionParameterDefinitions*>(other.parameters->copy())),
	scopeBlock(static_cast<CatScopeBlock*>(other.scopeBlock->copy())),
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


CatASTNode* jitcat::AST::CatFunctionDefinition::copy() const
{
	return new CatFunctionDefinition(*this);
}


void jitcat::AST::CatFunctionDefinition::print() const
{
	type->print();
	CatLog::log(" ", name, "(");
	parameters->print();
	CatLog::log(")");
	scopeBlock->print();
}


CatASTNodeType jitcat::AST::CatFunctionDefinition::getNodeType() const
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
		parametersScopeId = compileTimeContext->addScope(parameters->getCustomType(), nullptr, false);
	}
	CatScope* previousScope = compileTimeContext->getCurrentScope();
	compileTimeContext->setCurrentScope(this);
	if (!parameters->typeCheck(compileTimeContext, errorManager, this))
	{
		compileTimeContext->setCurrentScope(previousScope);
		compileTimeContext->removeScope(parametersScopeId);
		return false;
	}
	compileTimeContext->setCurrentFunction(this);
	for (int i = 0; i < parameters->getNumParameters(); i++)
	{
		parameterAssignables.emplace_back(new CatIdentifier(parameters->getParameterName(i), parameters->getParameterLexeme(i)));
		parameterAssignables.back()->typeCheck(compileTimeContext, errorManager, this);
	}
	if (compileTimeContext->getCurrentClass() != nullptr && Tools::equalsWhileIgnoringCase(name, "init"))
	{
		visibility = MemberVisibility::Constructor;
		//init function is a special case. We should call the auto generated __init function first.
		CatArgumentList* arguments = new CatArgumentList(lexeme);
		CatScopeRoot* scopeRoot = new CatScopeRoot(compileTimeContext->getCurrentClass()->getScopeId(), lexeme);
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
		if (compileTimeContext->findMemberFunction(this, existingFunctionScopeId) != nullptr && existingFunctionScopeId == currentScope->getScopeId())
		{
			errorManager->compiledWithError(Tools::append("A member function with the same signature and name \"", name, "\" already exists."), this, compileTimeContext->getContextName(), nameLexeme);
			return false;
		}
		CatGenericType thisType(compileTimeContext->getScopeType(currentScope->getScopeId()), false, false);
		memberFunctionInfo = currentScope->getCustomType()->addMemberFunction(name, thisType, this);
		memberFunctionInfo->visibility = visibility;
	}
	return true;
}


std::any jitcat::AST::CatFunctionDefinition::executeFunctionWithPack(CatRuntimeContext* runtimeContext, CatScopeID packScopeId)
{
	std::any result = scopeBlock->execute(runtimeContext);
	if (epilogBlock != nullptr)
	{
		epilogBlock->execute(runtimeContext);
	}
	runtimeContext->setReturning(false);
	runtimeContext->removeScope(packScopeId);
	return result;
}


std::any jitcat::AST::CatFunctionDefinition::executeFunctionWithArguments(CatRuntimeContext* runtimeContext, const std::vector<std::any>& arguments)
{
	assert(parameterAssignables.size() == arguments.size());
	CatScopeID scopeId = InvalidScopeID;
	unsigned char* scopeMem = nullptr;
	if (parameters->getNumParameters() > 0)
	{
		scopeMem = static_cast<unsigned char*>(alloca(parameters->getCustomType()->getTypeSize()));
		if constexpr (Configuration::logJitCatObjectConstructionEvents)
		{
			if (parameters->getCustomType()->getTypeSize() > 0)
			{
				std::cout << "(CatFunctionDefinition::executeFunctionWithArguments) Stack-allocated buffer of size " << std::dec << parameters->getCustomType()->getTypeSize() << ": " << std::hex << reinterpret_cast<uintptr_t>(scopeMem) << "\n";
			}
		}
		parameters->getCustomType()->placementConstruct(scopeMem, parameters->getCustomType()->getTypeSize());

		scopeId = pushScope(runtimeContext, scopeMem);
		//The scopeId should match the scopeId that was obtained during type checking.
		assert(scopeId == parametersScopeId);

		int i = 0;
		for (auto& iter : parameterAssignables)
		{
			std::any target = iter->executeAssignable(runtimeContext);
			ASTHelper::doAssignment(target, arguments[i], iter->getAssignableType(), iter->getType());
			i++;
		}
	}
	std::any result = executeFunctionWithPack(runtimeContext, scopeId);
	if (scopeMem != nullptr)
	{
		parameters->getCustomType()->placementDestruct(scopeMem, parameters->getCustomType()->getTypeSize());
	}
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

const CatGenericType& jitcat::AST::CatFunctionDefinition::getParameterType(int index) const
{
	return parameters->getParameterType(index)->getType();
}


/*const CatTypeNode* jitcat::AST::CatFunctionDefinition::getParameterType(int index) const
{
	return parameters->getParameterType(index);
}*/


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


const std::string& jitcat::AST::CatFunctionDefinition::getLowerCaseFunctionName() const
{
	return lowerCaseName;
}


const std::string & jitcat::AST::CatFunctionDefinition::getFunctionName() const
{
	return name;
}


CatScopeBlock* jitcat::AST::CatFunctionDefinition::getScopeBlock() const
{
	return scopeBlock.get();
}


CatScopeBlock* jitcat::AST::CatFunctionDefinition::getEpilogBlock() const
{
	return epilogBlock.get();
}


CatScopeBlock* jitcat::AST::CatFunctionDefinition::getOrCreateEpilogBlock(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (epilogBlock == nullptr)
	{
		epilogBlock = std::make_unique<CatScopeBlock>(std::vector<CatStatement*>(), nameLexeme);
		epilogBlock->typeCheck(compileTimeContext, errorManager, errorContext);
	}
	return epilogBlock.get();
}


CatScopeID CatFunctionDefinition::getScopeId() const
{
	return parametersScopeId;
}


Reflection::CustomTypeInfo* CatFunctionDefinition::getCustomType()
{
	return parameters->getCustomType();
}


CatScopeID jitcat::AST::CatFunctionDefinition::pushScope(CatRuntimeContext* runtimeContext, unsigned char* instance)
{
	return runtimeContext->addScope(parameters->getCustomType(), instance, false);
}
