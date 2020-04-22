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
		TypeMemberInfo(const std::string& memberName, const CatGenericType& type): memberName(memberName), catType(type), visibility(MemberVisibility::Public) {}
		virtual ~TypeMemberInfo() {};
		inline virtual std::any getMemberReference(unsigned char* base);
		inline virtual std::any getAssignableMemberReference(unsigned char* base);
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const;
		inline virtual llvm::Value* generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const;
		inline virtual bool isDeferred() const { return false; }
		inline virtual unsigned long long getOrdinal() const { return 0;}

		TypeMemberInfo* toDeferredTypeMemberInfo(TypeMemberInfo* baseMember);

		CatGenericType catType;
		MemberVisibility visibility;

		std::string memberName;
	};
};