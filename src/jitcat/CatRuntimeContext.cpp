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
#include "jitcat/Tools.h"

#include <cassert>
#include <functional>
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
		cloned->addScope(iter->scopeObject.getObjectType(), reinterpret_cast<unsigned char*>(iter->scopeObject.get()), iter->isStatic);
	}
	for (auto& iter : staticScopes)
	{
		cloned->addScope(iter->scopeObject.getObjectType(), reinterpret_cast<unsigned char*>(iter->scopeObject.get()), iter->isStatic);
	}
	#ifdef ENABLE_LLVM
		//This cannot be put inside a if constexpr unfortunately
		cloned->codeGenerator = codeGenerator;
	#endif
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
		assert(id >= 0 && id < static_cast<CatScopeID>(staticScopes.size()));
		staticScopes.erase(staticScopes.begin() + id);
	}
	else if (id != InvalidScopeID)
	{
		id += currentStackFrameOffset;
		assert(id >= 0 && id < static_cast<CatScopeID>(scopes.size()));
		scopes.erase(scopes.begin() + id);
	}
}


void CatRuntimeContext::setScopeObject(CatScopeID id, unsigned char* scopeObject)
{
	Scope* scope = getScope(id);
	scope->scopeObject.setReflectable(scopeObject, getScopeType(id));
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
	return scope->scopeObject.getObjectType();
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
		TypeMemberInfo* memberInfo = scopes[i]->scopeObject.getObjectType()->getMemberInfo(lowercaseName);
		if (memberInfo != nullptr)
		{
			scopeId = i;
			return memberInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		TypeMemberInfo* memberInfo = staticScopes[i]->scopeObject.getObjectType()->getMemberInfo(lowercaseName);
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
		StaticMemberInfo* staticMemberInfo = scopes[i]->scopeObject.getObjectType()->getStaticMemberInfo(lowercaseName);
		if (staticMemberInfo != nullptr)
		{
			scopeId = i;
			return staticMemberInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		StaticMemberInfo* staticMemberInfo = staticScopes[i]->scopeObject.getObjectType()->getStaticMemberInfo(lowercaseName);
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
		StaticConstMemberInfo* staticConstMemberInfo = scopes[i]->scopeObject.getObjectType()->getStaticConstMemberInfo(lowercaseName);
		if (staticConstMemberInfo != nullptr)
		{
			scopeId = i;
			return staticConstMemberInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		StaticConstMemberInfo* staticConstMemberInfo = staticScopes[i]->scopeObject.getObjectType()->getStaticConstMemberInfo(lowercaseName);
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
		MemberFunctionInfo* memberFunctionInfo = scopes[i]->scopeObject.getObjectType()->getFirstMemberFunctionInfo(lowercaseName);
		if (memberFunctionInfo != nullptr)
		{
			scopeId = i;
			return memberFunctionInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		MemberFunctionInfo* memberFunctionInfo = staticScopes[i]->scopeObject.getObjectType()->getFirstMemberFunctionInfo(lowercaseName);
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
		MemberFunctionInfo* memberFunctionInfo = scopes[i]->scopeObject.getObjectType()->getMemberFunctionInfo(functionSignature);
		if (memberFunctionInfo != nullptr)
		{
			scopeId = i;
			return memberFunctionInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		MemberFunctionInfo* memberFunctionInfo = staticScopes[i]->scopeObject.getObjectType()->getMemberFunctionInfo(functionSignature);
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
		StaticFunctionInfo* memberFunctionInfo = scopes[i]->scopeObject.getObjectType()->getStaticMemberFunctionInfo(functionSignature);
		if (memberFunctionInfo != nullptr)
		{
			scopeId = i;
			return memberFunctionInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		StaticFunctionInfo* memberFunctionInfo = staticScopes[i]->scopeObject.getObjectType()->getStaticMemberFunctionInfo(functionSignature);
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
		TypeInfo* scopeInfo = scopes[i]->scopeObject.getObjectType()->getTypeInfo(lowercaseName);
		if (scopeInfo != nullptr)
		{
			scopeId = i;
			return scopeInfo;
		}
	}
	for (int i = (int)staticScopes.size() - 1; i >= 0; i--)
	{
		TypeInfo* scopeInfo = staticScopes[i]->scopeObject.getObjectType()->getTypeInfo(lowercaseName);
		if (scopeInfo != nullptr)
		{
			scopeId = InvalidScopeID - i - 1;
			return scopeInfo;
		}
	}
	return nullptr;
}


void CatRuntimeContext::setCodeGenerator(std::shared_ptr<LLVM::LLVMCodeGenerator> codeGenerator_)
{
	#ifdef ENABLE_LLVM
		codeGenerator = codeGenerator_;
	#endif
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


std::size_t CatRuntimeContext::getContextHash() const
{
	std::hash<std::string> stringHasher;
	std::size_t totalHash = 0;
	for (const auto& iter : scopes)
	{
		totalHash = Tools::hashCombine(totalHash, stringHasher(iter->scopeObject.getObjectType()->getTypeName()));
	}
	for (const auto& iter : staticScopes)
	{
		//Same as normal scopes, except we reverse the bytes of the hash so that a normal scope is unlikely to match the hash of a global scope of the same type.
		totalHash = Tools::hashCombine(totalHash, Tools::reverseBytes(stringHasher(iter->scopeObject.getObjectType()->getTypeName())));
	}
	return totalHash;
}


void CatRuntimeContext::setPrecompilationContext(std::shared_ptr<PrecompilationContext> precompilationContext_)
{
	precompilationContext = precompilationContext_;
}


std::shared_ptr<PrecompilationContext> CatRuntimeContext::getPrecompilationContext() const
{
	return precompilationContext;
}


CatScopeID CatRuntimeContext::createScope(unsigned char* scopeObject, TypeInfo* type, bool isStatic)
{
	Scope* scope = new Scope(type, scopeObject, isStatic);
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
	//A break / crash in this function usually means that the CatRuntimeContext that was provided when
	//calling the expression does not contain the same number of scopes as the CatRuntimeContext that
	//was used to compile the expression.
	if (scopeId < InvalidScopeID)
	{
		//Static scope
		scopeId = std::abs(scopeId) - 2;
		assert(scopeId >= 0 && scopeId < static_cast<CatScopeID>(staticScopes.size()));

		return staticScopes[scopeId].get();
	}
	else if (scopeId != InvalidScopeID)
	{
		scopeId += currentStackFrameOffset;
		assert(scopeId >= 0 && scopeId < static_cast<CatScopeID>(scopes.size()));
		return scopes[scopeId].get();
	}
	return nullptr;
}


CatRuntimeContext CatRuntimeContext::defaultContext("default", nullptr);