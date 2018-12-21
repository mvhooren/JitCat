/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatRuntimeContext.h"
#include "CustomTypeInfo.h"
#include "ErrorContext.h"
#include "ExpressionErrorManager.h"
#include "LLVMCodeGenerator.h"

#include <cassert>
#include <sstream>


CatRuntimeContext::CatRuntimeContext(TypeInfo* globalType, TypeInfo* thisType, 
									 TypeInfo* customThisType, TypeInfo* customGlobalsType,
									 const std::string& contextName, bool isRuntimeContext, 
									 ExpressionErrorManager* errorManager):
	globalType(globalType),
	thisType(thisType),
	contextName(contextName),
	isRuntimeContext(isRuntimeContext),
	customThisType(customThisType),
	customGlobalsType(customGlobalsType),
	errorManager(errorManager),
	ownsErrorManager(false),
	codeGenerator(new LLVMCodeGenerator(contextName)),
	nextFunctionIndex(0)
{
	if (errorManager == nullptr)
	{
		ownsErrorManager = true;
		errorManager = new ExpressionErrorManager();
	}
}


CatRuntimeContext::~CatRuntimeContext()
{
	//unsigned int numListeners = destructionListeners.size();
	for (unsigned int i = 0; i < destructionListeners.size(); i++)
	{
		destructionListeners[i]->onContextDestroyed(this);
	}
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


bool CatRuntimeContext::isRunTimeContext() const
{
	return isRuntimeContext;
}


void CatRuntimeContext::registerDestructionListener(const CatRuntimeContextDestructionListener* destructionListener) const 
{
	std::size_t numListeners = destructionListeners.size();
	for (std::size_t i = 0; i < numListeners; i++)
	{
		if (destructionListeners[i] == destructionListener)
		{
			return;
		}
	}
	destructionListeners.push_back(destructionListener);
}


void CatRuntimeContext::deregisterDestructionListener(const CatRuntimeContextDestructionListener* destructionListener) const
{
	std::size_t numListeners = destructionListeners.size();
	for (std::size_t i = 0; i < numListeners; i++)
	{
		if (destructionListeners[i] == destructionListener)
		{
			destructionListeners.erase(destructionListeners.begin() + (int)i);
			return;
		}
	}
}


TypeInfo* CatRuntimeContext::getGlobalType() const
{
	return globalType;
}


TypeInfo* CatRuntimeContext::getThisType() const
{
	return thisType;
}


TypeInfo* CatRuntimeContext::getCustomThisType() const
{
	return customThisType;
}


TypeInfo* CatRuntimeContext::getCustomGlobalsType() const
{
	return customGlobalsType;
}


MemberReferencePtr CatRuntimeContext::getGlobalReference() const
{
	return globalReference;
}


MemberReferencePtr CatRuntimeContext::getThisReference() const
{
	return thisReference;
}


MemberReferencePtr CatRuntimeContext::getCustomThisReference() const
{
	return customThisReference;
}


MemberReferencePtr CatRuntimeContext::getRootReference(RootTypeSource source) const
{
	switch (source)
	{
		case RootTypeSource::Global:		return globalReference;
		case RootTypeSource::This:			return thisReference;
		case RootTypeSource::CustomThis:	return customThisReference;
		case RootTypeSource::CustomGlobals:return customGlobalsReference;
	}
	return MemberReferencePtr();
}


void CatRuntimeContext::setGlobalReference(MemberReferencePtr globalReference_)
{
	globalReference = globalReference_;
}


void CatRuntimeContext::setThisReference(MemberReferencePtr thisReference_)
{
	thisReference = thisReference_;
}


void CatRuntimeContext::setCustomThisReference(MemberReferencePtr customThisReference_)
{
	customThisReference = customThisReference_;
}


void CatRuntimeContext::setCustomGlobalsReference(MemberReferencePtr customGlobalsReference_)
{
	customGlobalsReference = customGlobalsReference_;
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


TypeMemberInfo* CatRuntimeContext::findIdentifier(const std::string& lowercaseName)
{
	TypeMemberInfo* memberInfo = nullptr;
	//First check if the variable name is a custom local
	findIdentifier(getCustomThisType(), lowercaseName, memberInfo);
	//Next, if the variable is not a custom local, check if the variable name is a normal local (via reflection)
	findIdentifier(getThisType(), lowercaseName, memberInfo);
	//Next, if the variable is not a local, check if the variable name is a custom global
	findIdentifier(getCustomGlobalsType(), lowercaseName, memberInfo);
	//Lastly, if the variable is not a local, check if the variable name is a global
	findIdentifier(getGlobalType(), lowercaseName, memberInfo);

	return memberInfo;
}


MemberFunctionInfo* CatRuntimeContext::findFunction(const std::string& lowercaseName, RootTypeSource& source)
{
	MemberFunctionInfo* memberFunctionInfo = nullptr;
	//First check if the variable name is a custom local
	findFunctionIdentifier(getCustomThisType(), lowercaseName, RootTypeSource::CustomThis, memberFunctionInfo, source);
	//Next, if the variable is not a custom local, check if the variable name is a normal local (via reflection)
	findFunctionIdentifier(getThisType(), lowercaseName, RootTypeSource::This, memberFunctionInfo, source);
	//Next, if the variable is not a local, check if the variable name is a custom global
	findFunctionIdentifier(getCustomGlobalsType(), lowercaseName, RootTypeSource::CustomGlobals, memberFunctionInfo, source);
	//Lastly, if the variable is not a local, check if the variable name is a global
	findFunctionIdentifier(getGlobalType(), lowercaseName, RootTypeSource::Global, memberFunctionInfo, source);

	return memberFunctionInfo;
}


LLVMCodeGenerator* CatRuntimeContext::getCodeGenerator() const
{
	return codeGenerator.get();
}


int CatRuntimeContext::getNextFunctionIndex()
{
	return nextFunctionIndex++;
}


void CatRuntimeContext::findIdentifier(TypeInfo* typeInfo, const std::string& lowercaseName, TypeMemberInfo*& memberInfo)
{
	if (memberInfo == nullptr && typeInfo != nullptr)
	{
		memberInfo = typeInfo->getMemberInfo(lowercaseName);
	}
}


void CatRuntimeContext::findFunctionIdentifier(TypeInfo* typeInfo, const std::string& lowercaseName, RootTypeSource source, MemberFunctionInfo*& functionInfo, RootTypeSource& sourceToSet)
{
	if (functionInfo == nullptr && typeInfo != nullptr)
	{
		sourceToSet = source;
		functionInfo = typeInfo->getMemberFunctionInfo(lowercaseName);
	}
}
