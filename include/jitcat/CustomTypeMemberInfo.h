/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::LLVM
{
	struct LLVMCompileTimeContext;
}

#include "jitcat/CustomTypeInstance.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/MemberInfo.h"

namespace jitcat::Reflection
{


	template<typename T>
	struct CustomBasicTypeMemberInfo: public TypeMemberInfo
	{
		CustomBasicTypeMemberInfo(const std::string& memberName, unsigned int memberOffset, const CatGenericType& type): TypeMemberInfo(memberName, type), memberOffset(memberOffset) {}

		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		unsigned long long getMemberOffset() const;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		void assign(std::any& base, const T& valueToSet);

		unsigned int memberOffset;
	};


	//Implements a TypeMemberInfo for class/struct types that are reflectable.
	struct CustomTypeObjectMemberInfo: public TypeMemberInfo
	{
		CustomTypeObjectMemberInfo(const std::string& memberName, unsigned int memberOffset, const CatGenericType& type): TypeMemberInfo(memberName, type), memberOffset(memberOffset) {}

		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		unsigned long long getMemberOffset() const;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		void assign(std::any& base, std::any& valueToSet);
	
		unsigned int memberOffset;
	};

}//End namespace jitcat::Reflection

#include "jitcat/CustomTypeMemberInfoHeaderImplementation.h"
