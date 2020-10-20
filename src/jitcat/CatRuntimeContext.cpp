/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/ErrorContext.h"
#include "jitcat/ExpressionErrorManager.h"
#ifdef ENABLE_LLVM
#include "jitcat/LLVMCodeGenerator.h"
#endif
#include "jitcat/ObjectInstance.h"

#include <cassert>
#include <sstream>

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;

CatRuntimeContext::CatRuntimeContext(const std::string& contextName, ExpressionErrorManager* errorManager):
	nextFunctionIndex(0),
	currentFunctionDefinition(nullptr),
	currentClassDefinition(nullptr),
	currentScope(nullptr),
	returning(false),
	ownsErrorManager(false),
	errorManager(errorManager),
	contextName(contextName),
	currentStackFrameOffset(0)
#ifdef ENABLE_LLVM
	,codeGenerator(nullptr)
#endif
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


std::unique_ptr<CatRuntimeContext> jitcat::CatRuntimeContext::clone() const
{
	std::unique_ptr<CatRuntimeContext> cloned = std::make_unique<CatRuntimeContext>(contextName, ownsErrorManager ? nullptr : errorManager);
	for (auto& iter : scopes)
	{
		cloned->addScope(iter->scopeType, reinterpret_cast<unsigned char*>(iter->scopeObject.get()), iter->isStatic);
	}
	for (auto& iter : staticScopes)
	{
		cloned->addScope(iter->scopeType, reinterpret_cast<unsigned char*>(iter->scopeObject.get()), iter->isStatic);
	}
	if constexpr (Configuration::enableLLVM)
	{
		cloned->codeGenerator = codeGenerator;
	}
	for (auto& iter : errorContextStack)
	{
		cloned->errorContextStack.push_back(iter);
	}
	cloned->currentScope = currentScope;
	return cloned;
}


const char* jitcat::CatRuntimeContext::getTypeName()
{
	return "CatRuntimeContext";
}


void CatRuntimeContext::reflect(jitcat::Reflection::ReflectedTypeInfo& typeInfo)
{
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


CatScopeID CatRuntimeContext::addScope(TypeInfo* typeInfo, unsigned char* scopeObject, bool isStatic)
{
	assert(typeInfo != nullptr);
	//If this is a static scope, scopeObject must not be nullptr.
	assert(!isStatic || scopeObject != nullptr);
	return createScope(scopeObject, typeInfo, isStatic);
}


CatScopeID CatRuntimeContext::addScope(const ObjectInstance& objectInstance, bool isStatic)
{
	return addScope(objectInstance.getType(), objectInstance.getObject(), isStatic);
}


void jitcat::CatRuntimeContext::pushStackFrame()
{
	currentStackFrameOffset = (int)scopes.size();
	stackFrameOffsets.push_back(currentStackFrameOffset);
}


void jitcat::CatRuntimeContext::popStackFrame()
{
	stackFrameOffsets.pop_back();
	if (stackFrameOffsets.size() > 0)
	{
		currentStackFrameOffset = stackFrameOffsets.back();
	}
	else
	{
		currentStackFrameOffset = 0;
	}
}


int CatRuntimeContext::getNumScopes() const
{
	return (int)scopes.size();
}


void CatRuntimeContext::removeScope(CatScopeID id)
{
	if (id < InvalidScopeID)
	{
		//Static scope
		id = std::abs(id) - 2;
		assert(id >= 0 && id < staticScopes.size());
		staticScopes.erase(staticScopes.begin() + id);
	}
	else if (id != InvalidScopeID)
	{
		id += currentStackFrameOffset;
		assert(id >= 0 && id < scopes.size());
		scopes.erase(scopes.begin() + id);
	}
}


void CatRuntimeContext::setScopeObject(CatScopeID id, unsigned char* scopeObject)
{
	Scope* scope = getScope(id);
	scope->scopeObject = reinterpret_cast<Reflectable*>(scopeObject);
}


bool CatRuntimeContext::isStaticScope(CatScopeID id) const
{
	Scope* scope = getScope(id);
	return scope->isStatic;
}


unsigned char* CatRuntimeContext::getScopeObject(CatScopeID id) const
{
	Scope* scope = getScope(id);
	return reinterpret_cast<unsigned char*>(scope->scopeObject.get());
}


TypeInfo* CatRuntimeContext::getScopeType(CatScopeID id) const
{
	Scope* scope = getScope(id);
	return scope->scopeType;
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
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		TypeMemberInfo* memberInfo = staticScopes[i]->scopeType->getMemberInfo(lowercaseName);
		if (memberInfo != nullptr)
		{
			scopeId = InvalidScopeID - i - 1;
			return memberInfo;
		}
	}
	return nullptr;
}


Reflection::StaticMemberInfo* jitcat::CatRuntimeContext::findStaticVariable(const std::string& lowercaseName, CatScopeID& scopeId)
{
	for (int i = (int)scopes.size() - 1; i >= 0; i--)
	{
		StaticMemberInfo* staticMemberInfo = scopes[i]->scopeType->getStaticMemberInfo(lowercaseName);
		if (staticMemberInfo != nullptr)
		{
			scopeId = i;
			return staticMemberInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		StaticMemberInfo* staticMemberInfo = staticScopes[i]->scopeType->getStaticMemberInfo(lowercaseName);
		if (staticMemberInfo != nullptr)
		{
			scopeId = InvalidScopeID - i - 1;
			return staticMemberInfo;
		}
	}
	return nullptr;
}


Reflection::StaticConstMemberInfo* jitcat::CatRuntimeContext::findStaticConstant(const std::string& lowercaseName, CatScopeID& scopeId)
{
	for (int i = (int)scopes.size() - 1; i >= 0; i--)
	{
		StaticConstMemberInfo* staticConstMemberInfo = scopes[i]->scopeType->getStaticConstMemberInfo(lowercaseName);
		if (staticConstMemberInfo != nullptr)
		{
			scopeId = i;
			return staticConstMemberInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		StaticConstMemberInfo* staticConstMemberInfo = staticScopes[i]->scopeType->getStaticConstMemberInfo(lowercaseName);
		if (staticConstMemberInfo != nullptr)
		{
			scopeId = InvalidScopeID - i - 1;
			return staticConstMemberInfo;
		}
	}
	return nullptr;
}


MemberFunctionInfo* CatRuntimeContext::findFirstMemberFunction(const std::string& lowercaseName, CatScopeID& scopeId)
{
	for (int i = (int)scopes.size() - 1; i >= 0; i--)
	{
		MemberFunctionInfo* memberFunctionInfo = scopes[i]->scopeType->getFirstMemberFunctionInfo(lowercaseName);
		if (memberFunctionInfo != nullptr)
		{
			scopeId = i;
			return memberFunctionInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		MemberFunctionInfo* memberFunctionInfo = staticScopes[i]->scopeType->getFirstMemberFunctionInfo(lowercaseName);
		if (memberFunctionInfo != nullptr)
		{
			scopeId = InvalidScopeID - i - 1;
			return memberFunctionInfo;
		}
	}
	return nullptr;
}


Reflection::MemberFunctionInfo* jitcat::CatRuntimeContext::findMemberFunction(const FunctionSignature* functionSignature, CatScopeID& scopeId)
{
	for (int i = (int)scopes.size() - 1; i >= 0; i--)
	{
		MemberFunctionInfo* memberFunctionInfo = scopes[i]->scopeType->getMemberFunctionInfo(functionSignature);
		if (memberFunctionInfo != nullptr)
		{
			scopeId = i;
			return memberFunctionInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		MemberFunctionInfo* memberFunctionInfo = staticScopes[i]->scopeType->getMemberFunctionInfo(functionSignature);
		if (memberFunctionInfo != nullptr)
		{
			scopeId = InvalidScopeID - i - 1;
			return memberFunctionInfo;
		}
	}
	return nullptr;
}


Reflection::StaticFunctionInfo* jitcat::CatRuntimeContext::findStaticFunction(const Reflection::FunctionSignature* functionSignature, CatScopeID& scopeId)
{
	for (int i = (int)scopes.size() - 1; i >= 0; i--)
	{
		StaticFunctionInfo* memberFunctionInfo = scopes[i]->scopeType->getStaticMemberFunctionInfo(functionSignature);
		if (memberFunctionInfo != nullptr)
		{
			scopeId = i;
			return memberFunctionInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		StaticFunctionInfo* memberFunctionInfo = staticScopes[i]->scopeType->getStaticMemberFunctionInfo(functionSignature);
		if (memberFunctionInfo != nullptr)
		{
			scopeId = InvalidScopeID - i - 1;
			return memberFunctionInfo;
		}
	}
	return nullptr;
}


Reflection::TypeInfo* jitcat::CatRuntimeContext::findType(const std::string& lowercaseName, CatScopeID& scopeId)
{
	for (int i = (int)scopes.size() - 1; i >= 0; i--)
	{
		TypeInfo* scopeInfo = scopes[i]->scopeType->getTypeInfo(lowercaseName);
		if (scopeInfo != nullptr)
		{
			scopeId = i;
			return scopeInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		TypeInfo* scopeInfo = staticScopes[i]->scopeType->getTypeInfo(lowercaseName);
		if (scopeInfo != nullptr)
		{
			scopeId = InvalidScopeID - i - 1;
			return scopeInfo;
		}
	}
	return nullptr;
}


std::shared_ptr<LLVMCodeGenerator> CatRuntimeContext::getCodeGenerator()
{
#ifdef ENABLE_LLVM
	if (codeGenerator == nullptr)
	{
		codeGenerator = std::make_shared<LLVMCodeGenerator>(contextName);
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


void jitcat::CatRuntimeContext::setCurrentClass(AST::CatClassDefinition* currentClass)
{
	currentClassDefinition = currentClass;
}


AST::CatClassDefinition* jitcat::CatRuntimeContext::getCurrentClass() const
{
	return currentClassDefinition;
}


void jitcat::CatRuntimeContext::setCurrentScope(CatScope* scope)
{
	currentScope = scope;
}


CatScope* jitcat::CatRuntimeContext::getCurrentScope() const
{
	return currentScope;
}


unsigned char* jitcat::CatRuntimeContext::getCurrentScopeObject() const
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


std::any& jitcat::CatRuntimeContext::addTemporary(const std::any& value)
{
	temporaries.emplace_back(std::make_unique<std::any>(value));
	return *temporaries.back().get();
}


void jitcat::CatRuntimeContext::clearTemporaries()
{
	temporaries.clear();
}


CatScopeID CatRuntimeContext::createScope(unsigned char* scopeObject, TypeInfo* type, bool isStatic)
{
	Scope* scope = new Scope(type, reinterpret_cast<Reflectable*>(scopeObject), isStatic);
	if (!isStatic)
	{
		scopes.emplace_back(scope);
		return static_cast<CatScopeID>((int)scopes.size() - 1) - currentStackFrameOffset;
	}
	else
	{
		staticScopes.emplace_back(scope);
		return static_cast<CatScopeID>(InvalidScopeID - (int)staticScopes.size());
	}
}


CatRuntimeContext::Scope* jitcat::CatRuntimeContext::getScope(CatScopeID scopeId) const
{
	if (scopeId < InvalidScopeID)
	{
		//Static scope
		scopeId = std::abs(scopeId) - 2;
		assert(scopeId >= 0 && scopeId < staticScopes.size());

		return staticScopes[scopeId].get();
	}
	else if (scopeId != InvalidScopeID)
	{
		scopeId += currentStackFrameOffset;
		assert(scopeId >= 0 && scopeId < scopes.size());
		return scopes[scopeId].get();
	}
	return nullptr;
}


CatRuntimeContext CatRuntimeContext::defaultContext("default", nullptr);