/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

struct LLVMCompileTimeContext;

#include "CustomTypeInstance.h"
#include "LLVMForwardDeclares.h"
#include "MemberInfo.h"


template<typename T>
struct CustomBasicTypeMemberInfo: public TypeMemberInfo
{
	CustomBasicTypeMemberInfo(const std::string& memberName, unsigned int memberOffset, CatType type, bool isConst, bool isWritable): TypeMemberInfo(memberName, type, isConst, isWritable), memberOffset(memberOffset) {}

	inline virtual std::any getMemberReference(std::any& base) override final;
	inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCompileTimeContext* context) const override final;

	void assign(std::any& base, const T& valueToSet);

	unsigned int memberOffset;
};


//Implements a TypeMemberInfo for class/struct types that are reflectable.
struct CustomTypeObjectMemberInfo: public TypeMemberInfo
{
	CustomTypeObjectMemberInfo(const std::string& memberName, unsigned int memberOffset, TypeInfo* type, bool isConst): TypeMemberInfo(memberName, type, isConst, false), memberOffset(memberOffset) {}

	inline virtual std::any getMemberReference(std::any& base) override final;
	inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCompileTimeContext* context) const override final;
	
	void assign(std::any& base, std::any& valueToSet);
	
	unsigned int memberOffset;
};

#include "CustomTypeMemberInfoHeaderImplementation.h"