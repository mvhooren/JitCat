/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


class CatRuntimeContext;
class CustomTypeInfo;
class ErrorContext;
class ExpressionErrorManager;
class LLVMCodeGenerator;
struct MemberFunctionInfo;
class MemberReference;
class TypeInfo;
struct TypeMemberInfo;
#include "CatType.h"
#include "MemberReferencePtr.h"
#include "RootTypeSource.h"
#include "RuntimeContext.h"

#include <memory>
#include <string>
#include <vector>


class CatRuntimeContextDestructionListener
{
public:
	virtual void onContextDestroyed(CatRuntimeContext* context) const = 0;
};

//TODO: Split CatRuntimeContext up, adding a CatCompileTimeContext. CatRuntimeContext can be significantly simplified (maybe even just an array of "this" pointers).
class CatRuntimeContext: public RuntimeContext
{
public:
	CatRuntimeContext(TypeInfo* globalType, TypeInfo* thisType, 
					  TypeInfo* customThisType, TypeInfo* customGlobalsType,
					  const std::string& contextName, bool isRuntimeContext,
					  ExpressionErrorManager* errorManager);
	virtual ~CatRuntimeContext();
	//Returns the name of the context for logging of errors.
	std::string getContextName();

	bool isRunTimeContext() const;

	void registerDestructionListener(const CatRuntimeContextDestructionListener* destructionListener) const;
	void deregisterDestructionListener(const CatRuntimeContextDestructionListener* destructionListener) const;

	TypeInfo* getGlobalType() const;
	TypeInfo* getThisType() const;
	TypeInfo* getCustomThisType() const;
	TypeInfo* getCustomGlobalsType() const;

	MemberReferencePtr getGlobalReference() const;
	MemberReferencePtr getThisReference() const;
	MemberReferencePtr getCustomThisReference() const;
	MemberReferencePtr getRootReference(RootTypeSource source) const;

	void setGlobalReference(MemberReferencePtr globalReference_);
	void setThisReference(MemberReferencePtr thisReference_);
	void setCustomThisReference(MemberReferencePtr customThisReference_);
	void setCustomGlobalsReference(MemberReferencePtr customGlobalsReference_);

	ExpressionErrorManager* getErrorManager() const;
	void pushErrorContext(ErrorContext* context);
	void popErrorContext(ErrorContext* context);

	TypeMemberInfo* findIdentifier(const std::string& lowercaseName);
	MemberFunctionInfo* findFunction(const std::string& lowercaseName, RootTypeSource& source);

	LLVMCodeGenerator* getCodeGenerator() const;
	int getNextFunctionIndex();

private:
	void findIdentifier(TypeInfo* typeInfo, const std::string& lowercaseName, TypeMemberInfo*& memberInfo);
	void findFunctionIdentifier(TypeInfo* typeInfo, const std::string& lowercaseName, RootTypeSource source, MemberFunctionInfo*& functionInfo, RootTypeSource& sourceToSet);

private:
	int nextFunctionIndex;
	//These are not owned here
	TypeInfo* globalType;
	MemberReferencePtr globalReference;
	TypeInfo* thisType;
	MemberReferencePtr thisReference;
	TypeInfo* customThisType;
	MemberReferencePtr customThisReference;
	TypeInfo* customGlobalsType;
	MemberReferencePtr customGlobalsReference;

	bool ownsErrorManager;
	ExpressionErrorManager* errorManager;

	bool isRuntimeContext;

	std::string contextName;
	mutable std::vector<const CatRuntimeContextDestructionListener*> destructionListeners;

	std::unique_ptr<LLVMCodeGenerator> codeGenerator;

	std::vector<ErrorContext*> errorContextStack;
};