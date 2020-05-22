/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/MemberVisibility.h"

#include <string>

namespace jitcat::LLVM
{
	class LLVMCodeGeneratorHelper;
	struct LLVMCompileTimeContext;
}


namespace jitcat::Reflection
{
	struct TypeMemberInfo
	{
		TypeMemberInfo(): visibility(MemberVisibility::Private) {}
		TypeMemberInfo(const std::string& memberName, const CatGenericType& type): catType(type), visibility(MemberVisibility::Public), memberName(memberName)  {}
		virtual ~TypeMemberInfo() {};
		virtual std::any getMemberReference(unsigned char* base);
		virtual std::any getAssignableMemberReference(unsigned char* base);
		virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const;
		virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const;
		virtual llvm::Value* generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const;
		virtual bool isDeferred() const { return false; }
		virtual unsigned long long getOrdinal() const { return 0;}

		TypeMemberInfo* toDeferredTypeMemberInfo(TypeMemberInfo* baseMember);

		CatGenericType catType;
		MemberVisibility visibility;

		std::string memberName;
	};
};