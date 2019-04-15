/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeInstance.h"
#include "jitcat/ErrorContext.h"
#include "jitcat/ExpressionErrorManager.h"
#ifdef ENABLE_LLVM
#include "jitcat/LLVMCodeGenerator.h"
#endif

#include <cassert>
#include <sstream>

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;

CatRuntimeContext::CatRuntimeContext(const std::string& contextName, ExpressionErrorManager* errorManager):
	contextName(contextName),
	errorManager(errorManager),
	ownsErrorManager(false),
#ifdef ENABLE_LLVM
	codeGenerator(nullptr),
#endif
	nextFunctionIndex(0),
	currentFunctionDefinition(nullptr),
	currentScope(nullptr),
	returning(false)
{
	if (errorManager == nullptr)
	{
		ownsErrorManager = true;
		this->errorManager = new ExpressionErrorManager();
	}
}


CatRuntimeContext::~CatRuntimeContext()
{
	if (ownsErrorManager)
	{
		delete errorManager;
	}
}


std::string CatRuntimeContext::getContextName()
{
	std::stringstream stream;
	for (ErrorContext* errorContext : errorContextStack)
	{
		stream << errorContext->getContextDescription() << " ";
	}
	if (errorContextStack.size() != 0)
	{
		return contextName + " " + stream.str();
	}
	else
	{
		return contextName;
	}
}


CatScopeID CatRuntimeContext::addScope(TypeInfo* typeInfo, Reflectable* scopeObject, bool isStatic)
{
	return createScope(scopeObject, typeInfo, isStatic);
}


CatScopeID CatRuntimeContext::addCustomTypeScope(CustomTypeInfo* typeInfo, CustomTypeInstance* scopeObject, bool isStatic)
{
	assert(typeInfo != nullptr);
	//If this is a static scope, scopeObject must not be nullptr.
	assert(!isStatic || scopeObject != nullptr);
	if (scopeObject != nullptr)
	{
		//The provided scopeObject must be of the same type as the typeInfo.
		assert(scopeObject->typeInfo == typeInfo);
	}
	return createScope(scopeObject, typeInfo, isStatic);	
}


int CatRuntimeContext::getNumScopes() const
{
	return (int)scopes.size();
}


void CatRuntimeContext::removeScope(CatScopeID id)
{
	if (id != InvalidScopeID)
	{
		assert(id >= 0 && id < scopes.size());
		scopes.erase(scopes.begin() + id);
	}
}


void CatRuntimeContext::setScopeObject(CatScopeID id, Reflectable* scopeObject)
{
	assert(id >= 0 && id < scopes.size());
	Scope* scope = scopes[id].get();
	if (scope->scopeType->isCustomType())
	{
		assert(static_cast<CustomTypeInstance*>(scopeObject)->typeInfo == scope->scopeType);
	}
	scope->scopeObject = scopeObject;
}


bool CatRuntimeContext::isStaticScope(CatScopeID id) const
{
	assert(id >= 0 && id < scopes.size());
	return scopes[id]->isStatic;
}


Reflectable* CatRuntimeContext::getScopeObject(CatScopeID id) const
{
	assert(id >= 0 && id < scopes.size());
	return scopes[id]->scopeObject.get();
}


TypeInfo* CatRuntimeContext::getScopeType(CatScopeID id) const
{
	assert(id >= 0 && id < scopes.size());
	return scopes[id]->scopeType;
}


ExpressionErrorManager* CatRuntimeContext::getErrorManager() const
{
	return errorManager;
}


void CatRuntimeContext::pushErrorContext(ErrorContext* context)
{
	errorContextStack.push_back(context);
}


void CatRuntimeContext::popErrorContext(ErrorContext* context)
{
	if (errorContextStack.back() == context)
	{
		errorContextStack.pop_back();
	}
	else
	{
		//This means that contexts were pushed in a different order than they were popped!
		assert(false);
	}
}


TypeMemberInfo* CatRuntimeContext::findVariable(const std::string& lowercaseName, CatScopeID& scopeId)
{
	for (int i = (int)scopes.size() - 1; i >= 0; i--)
	{
		TypeMemberInfo* memberInfo = scopes[i]->scopeType->getMemberInfo(lowercaseName);
		if (memberInfo != nullptr)
		{
			scopeId = i;
			return memberInfo;
		}
	}
	return nullptr;
}


MemberFunctionInfo* CatRuntimeContext::findFunction(const std::string& lowercaseName, CatScopeID& scopeId)
{
	for (int i = (int)scopes.size() - 1; i >= 0; i--)
	{
		MemberFunctionInfo* memberFunctionInfo = scopes[i]->scopeType->getMemberFunctionInfo(lowercaseName);
		if (memberFunctionInfo != nullptr)
		{
			scopeId = i;
			return memberFunctionInfo;
		}
	}
	return nullptr;
}


std::shared_ptr<LLVMCodeGenerator> CatRuntimeContext::getCodeGenerator()
{
#ifdef ENABLE_LLVM
	if (codeGenerator == nullptr)
	{
		codeGenerator.reset(new LLVMCodeGenerator(contextName));
	}
	return codeGenerator;
#else
	return nullptr;
#endif
}


int CatRuntimeContext::getNextFunctionIndex()
{
	return nextFunctionIndex++;
}


void jitcat::CatRuntimeContext::setCurrentFunction(AST::CatFunctionDefinition* function)
{
	currentFunctionDefinition = function;
}


AST::CatFunctionDefinition* jitcat::CatRuntimeContext::getCurrentFunction() const
{
	return currentFunctionDefinition;
}


void jitcat::CatRuntimeContext::setCurrentScope(CatScope* scope)
{
	currentScope = scope;
}


CatScope* jitcat::CatRuntimeContext::getCurrentScope() const
{
	return currentScope;
}


Reflection::Reflectable* jitcat::CatRuntimeContext::getCurrentScopeObject() const
{
	if (currentScope != nullptr)
	{
		return getScopeObject(currentScope->getScopeId());
	}
	return nullptr;
}


bool jitcat::CatRuntimeContext::getIsReturning() const
{
	return returning;
}


void jitcat::CatRuntimeContext::setReturning(bool isReturning)
{
	returning = isReturning;
}


CatScopeID CatRuntimeContext::createScope(Reflectable* scopeObject, TypeInfo* type, bool isStatic)
{
	Scope* scope = new Scope();
	scope->isStatic = isStatic;
	scope->scopeObject = scopeObject;
	scope->scopeType = type;
	scopes.emplace_back(scope);
	return static_cast<CatScopeID>((int)scopes.size() - 1);
}


CatRuntimeContext CatRuntimeContext::defaultContext("default", nullptr);