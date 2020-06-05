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
#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeMemberFunctionInfo.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/FunctionNameMangler.h"
#include "jitcat/ObjectInstance.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"

#include <cassert>
#include <sstream>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatFunctionDefinition::CatFunctionDefinition(CatTypeNode* type, const std::string& name, const Tokenizer::Lexeme& nameLexeme, CatFunctionParameterDefinitions* parameters, CatScopeBlock* scopeBlock, const Tokenizer::Lexeme& lexeme):
	CatDefinition(lexeme),
	name(name),
	lowerCaseName(Tools::toLowerCase(name)),
	nameLexeme(nameLexeme),
	type(type),
	parameters(parameters),
	visibility(MemberVisibility::Public),
	parametersScopeId(InvalidScopeID),
	scopeBlock(scopeBlock),
	parentClass(nullptr),
	memberFunctionInfo(nullptr)
{
}


CatFunctionDefinition::CatFunctionDefinition(const CatFunctionDefinition& other):
	CatDefinition(other),
	name(other.name),
	lowerCaseName(other.lowerCaseName),
	nameLexeme(other.nameLexeme),
	type(static_cast<CatTypeNode*>(other.type->copy())),
	parameters(static_cast<CatFunctionParameterDefinitions*>(other.parameters->copy())),
	visibility(other.visibility),
	parametersScopeId(InvalidScopeID),
	scopeBlock(static_cast<CatScopeBlock*>(other.scopeBlock->copy())),
	memberFunctionInfo(nullptr)
{
}


CatFunctionDefinition::~CatFunctionDefinition()
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
	}
}


CatASTNode* CatFunctionDefinition::copy() const
{
	return new CatFunctionDefinition(*this);
}


void CatFunctionDefinition::print() const
{
	type->print();
	CatLog::log(" ", name, "(");
	parameters->print();
	CatLog::log(")");
	scopeBlock->print();
}


CatASTNodeType CatFunctionDefinition::getNodeType() const
{
	return CatASTNodeType::FunctionDefinition;
}


bool CatFunctionDefinition::typeCheck(CatRuntimeContext* compileTimeContext)
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
		CatArgumentList* arguments = new CatArgumentList(lexeme, std::vector<CatTypedExpression*>());
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
	ASTHelper::updatePointerIfChanged(scopeBlock, scopeBlock->constCollapse(compileTimeContext, errorManager, this));
	compileTimeContext->removeScope(parametersScopeId);
	compileTimeContext->setCurrentScope(previousScope);
	compileTimeContext->setCurrentFunction(nullptr);
	bool unreachableCodeDetected = false;
	allControlPathsReturn = scopeBlock->checkControlFlow(compileTimeContext, errorManager, this, unreachableCodeDetected);
	if(!type->getType().isVoidType() && !getAllControlPathsReturn())
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
	updateMangledName();
	return true;
}


std::any CatFunctionDefinition::executeFunctionWithPack(CatRuntimeContext* runtimeContext, CatScopeID packScopeId)
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


std::any CatFunctionDefinition::executeFunctionWithArguments(CatRuntimeContext* runtimeContext, const std::vector<std::any>& arguments)
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

		scopeId = pushScope(runtimeContext, scopeMem);
		//The scopeId should match the scopeId that was obtained during type checking.
		assert(scopeId == parametersScopeId);

		auto& members = parameters->getCustomType()->getMembersByOrdinal();
		int index = 0;
		for (auto& iter : members)
		{
			const unsigned char* sourceBuffer = nullptr;
			std::size_t sourceBufferSize = 0;
			iter.second->catType.toBuffer(arguments[index], sourceBuffer, sourceBufferSize);
			assert(sourceBuffer != nullptr);
			iter.second->catType.copyConstruct(scopeMem + iter.first, iter.second->catType.getTypeSize(), sourceBuffer, sourceBufferSize);
			++index;
		}
	}
	std::any result = executeFunctionWithPack(runtimeContext, scopeId);
	if (scopeMem != nullptr)
	{
		parameters->getCustomType()->placementDestruct(scopeMem, parameters->getCustomType()->getTypeSize());
	}
	return result;
}


jitcat::Reflection::CustomTypeInfo* CatFunctionDefinition::getParametersType() const
{
	return parameters->getCustomType();
}


CatTypeNode* CatFunctionDefinition::getReturnTypeNode() const
{
	return type.get();
}


int CatFunctionDefinition::getNumParameters() const
{
	return parameters->getNumParameters();
}


const std::string& CatFunctionDefinition::getParameterName(int index) const
{
	return parameters->getParameterName(index);
}


const CatGenericType& CatFunctionDefinition::getParameterType(int index) const
{
	return parameters->getParameterType(index)->getType();
}


Reflection::MemberVisibility CatFunctionDefinition::getFunctionVisibility() const
{
	return visibility;
}


void CatFunctionDefinition::setFunctionVisibility(Reflection::MemberVisibility functionVisibility)
{
	visibility = functionVisibility;
	if (memberFunctionInfo != nullptr)
	{
		memberFunctionInfo->visibility = visibility;
	}
}


void CatFunctionDefinition::setParentClass(const CatClassDefinition* classDefinition)
{
	parentClass = classDefinition;
	updateMangledName();
}


const std::string& CatFunctionDefinition::getLowerCaseFunctionName() const
{
	return lowerCaseName;
}


const std::string & CatFunctionDefinition::getFunctionName() const
{
	return name;
}


const std::string& CatFunctionDefinition::getMangledFunctionName() const
{
	return mangledName;
}


CatScopeBlock* CatFunctionDefinition::getScopeBlock() const
{
	return scopeBlock.get();
}


CatScopeBlock* CatFunctionDefinition::getEpilogBlock() const
{
	return epilogBlock.get();
}


CatScopeBlock* CatFunctionDefinition::getOrCreateEpilogBlock(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
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


Reflection::CustomTypeInfo* CatFunctionDefinition::getCustomType() const
{
	return parameters->getCustomType();
}


bool CatFunctionDefinition::getAllControlPathsReturn() const
{
	return allControlPathsReturn.has_value() && allControlPathsReturn.value();
}


CatScopeID CatFunctionDefinition::pushScope(CatRuntimeContext* runtimeContext, unsigned char* instance)
{
	return runtimeContext->addScope(parameters->getCustomType(), instance, false);
}


void CatFunctionDefinition::updateMangledName()
{
	std::vector<CatGenericType> parameterTypes;
	for (int i = 0; i < parameters->getNumParameters(); ++i)
	{
		parameterTypes.push_back(parameters->getParameterType(i)->getType());
	}
	std::string qualifiedParent;
	if (parentClass != nullptr)
	{
		qualifiedParent = parentClass->getQualifiedName();
	}
	mangledName = FunctionNameMangler::getMangledFunctionName(type->getType(), name, parameterTypes, true, qualifiedParent);
}
