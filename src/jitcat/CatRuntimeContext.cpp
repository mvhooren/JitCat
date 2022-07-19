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
#include "jitcat/JitCat.h"
#ifdef ENABLE_LLVM
	#include "jitcat/LLVMCodeGenerator.h"
	#include "jitcat/LLVMJit.h"
#endif
#include "jitcat/ObjectInstance.h"
#include "jitcat/PrecompilationContext.h"
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


std::unique_ptr<CatRuntimeContext> CatRuntimeContext::clone() const
{
	std::unique_ptr<CatRuntimeContext> cloned = std::make_unique<CatRuntimeContext>(contextName, ownsErrorManager ? nullptr : errorManager);
	for (auto& iter : scopes)
	{
		cloned->addDynamicScope(iter->scopeObject.getObjectType(), reinterpret_cast<unsigned char*>(iter->scopeObject.get()));
	}
	for (auto& iter : staticScopes)
	{
		cloned->createStaticScope(reinterpret_cast<unsigned char*>(iter->scopeObject.get()), iter->scopeObject.getObjectType(), iter->scopeName);
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


const char* CatRuntimeContext::getTypeName()
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


CatScopeID CatRuntimeContext::addDynamicScope(TypeInfo* typeInfo, unsigned char* scopeObject)
{
	assert(typeInfo != nullptr);
	return createDynamicScope(scopeObject, typeInfo);
}


CatScopeID CatRuntimeContext::addDynamicScope(const ObjectInstance& objectInstance)
{
	return addDynamicScope(objectInstance.getType(), objectInstance.getObject());
}


CatScopeID CatRuntimeContext::addStaticScope(TypeInfo* typeInfo, unsigned char* scopeObject, const std::string& staticScopeUniqueName)
{
	assert(typeInfo != nullptr);
	assert(scopeObject != nullptr);
	return createStaticScope(scopeObject, typeInfo, getGlobalNameReference(staticScopeUniqueName));
}


CatScopeID CatRuntimeContext::addStaticScope(const ObjectInstance& objectInstance, const std::string& staticScopeUniqueName)
{
	return addStaticScope(objectInstance.getType(), objectInstance.getObject(), staticScopeUniqueName);
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


int CatRuntimeContext::getNumStaticScopes() const
{
	return (int)staticScopes.size();
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


const std::string_view jitcat::CatRuntimeContext::getScopeNameView(CatScopeID id) const
{
	Scope* scope = getScope(id);
	assert(scope != nullptr);
	return scope->scopeName;
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


Reflection::StaticConstMemberInfo* CatRuntimeContext::findStaticConstant(const std::string& lowercaseName, CatScopeID& scopeId)
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


Reflection::MemberFunctionInfo* CatRuntimeContext::findMemberFunction(const FunctionSignature* functionSignature, CatScopeID& scopeId)
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


Reflection::StaticFunctionInfo* CatRuntimeContext::findStaticFunction(const Reflection::FunctionSignature* functionSignature, CatScopeID& scopeId)
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


Reflection::TypeInfo* CatRuntimeContext::findType(const std::string& lowercaseName, CatScopeID& scopeId)
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
		codeGenerator = std::make_shared<LLVMCodeGenerator>(contextName, LLVM::LLVMJit::get().getJitTargetConfig());
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


void CatRuntimeContext::setCurrentFunction(AST::CatFunctionDefinition* function)
{
	currentFunctionDefinition = function;
}


AST::CatFunctionDefinition* CatRuntimeContext::getCurrentFunction() const
{
	return currentFunctionDefinition;
}


void jitcat::CatRuntimeContext::setCurrentClass(AST::CatClassDefinition* currentClass)
{
	currentClassDefinition = currentClass;
}


AST::CatClassDefinition* CatRuntimeContext::getCurrentClass() const
{
	return currentClassDefinition;
}


void CatRuntimeContext::setCurrentScope(CatScope* scope)
{
	currentScope = scope;
}


CatScope* CatRuntimeContext::getCurrentScope() const
{
	return currentScope;
}


unsigned char* CatRuntimeContext::getCurrentScopeObject() const
{
	if (currentScope != nullptr)
	{
		return getScopeObject(currentScope->getScopeId());
	}
	return nullptr;
}


bool CatRuntimeContext::getIsReturning() const
{
	return returning;
}


void CatRuntimeContext::setReturning(bool isReturning)
{
	returning = isReturning;
}


std::any& CatRuntimeContext::addTemporary(const std::any& value)
{
	temporaries.emplace_back(std::make_unique<std::any>(value));
	return *temporaries.back().get();
}


void CatRuntimeContext::clearTemporaries()
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


CatRuntimeContext& CatRuntimeContext::getDefaultContext()
{
	static CatRuntimeContext defaultContext("default", nullptr);
	return defaultContext;
}


CatScopeID CatRuntimeContext::createDynamicScope(unsigned char* scopeObject, TypeInfo* type)
{
	Scope* scope = new Scope(type, scopeObject, false, Tools::empty);
	scopes.emplace_back(scope);
	return static_cast<CatScopeID>((int)scopes.size() - 1) - currentStackFrameOffset;
}


CatScopeID CatRuntimeContext::createStaticScope(unsigned char* scopeObject, TypeInfo* type, const std::string_view& staticScopeUniqueName)
{
	Scope* scope = new Scope(type, scopeObject, true, staticScopeUniqueName);
	staticScopes.emplace_back(scope);
	if (JitCat::get()->getHasPrecompiledExpression())
	{
		JitCat::get()->setPrecompiledGlobalVariable(staticScopeUniqueName, scopeObject);
	}
	return static_cast<CatScopeID>(InvalidScopeID - (int)staticScopes.size());
}


CatRuntimeContext::Scope* CatRuntimeContext::getScope(CatScopeID scopeId) const
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


const std::string_view jitcat::CatRuntimeContext::getGlobalNameReference(const std::string& globalName)
{
	return JitCat::defineGlobalVariableName(globalName);
}


