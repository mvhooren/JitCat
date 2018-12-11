/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "BasicTypeMemberReference.h"
#include "CustomTypeInstance.h"
#include "LLVMForwardDeclares.h"
#include "MemberInfo.h"
#include "ObjectMemberReference.h"


template<typename T>
struct CustomBasicTypeMemberInfo: public TypeMemberInfo
{
	CustomBasicTypeMemberInfo(const std::string& memberName, unsigned int memberOffset, CatType type, bool isConst, bool isWritable): TypeMemberInfo(memberName, type, isConst, isWritable), memberOffset(memberOffset) {}

	inline virtual MemberReferencePtr getMemberReference(MemberReferencePtr& base) override final;
	inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCodeGeneratorHelper* generatorHelper) const override final;

	void assign(MemberReferencePtr& base, const T& valueToSet);

	unsigned int memberOffset;
};


//Implements a TypeMemberInfo for class/struct types that are reflectable.
struct CustomTypeObjectMemberInfo: public TypeMemberInfo
{
	CustomTypeObjectMemberInfo(const std::string& memberName, unsigned int memberOffset, TypeInfo* type, bool isConst): TypeMemberInfo(memberName, type, isConst, false), memberOffset(memberOffset) {}

	inline static Reflectable* getReflectable(MemberReferencePtr& reference);

	inline virtual MemberReferencePtr getMemberReference(MemberReferencePtr& base) override final;
	inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCodeGeneratorHelper* generatorHelper) const override final;
	
	void assign(MemberReferencePtr& base, MemberReferencePtr valueToSet);
	
	unsigned int memberOffset;
};

#include "CustomTypeMemberInfoHeaderImplementation.h"