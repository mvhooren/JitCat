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

#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/MemberInfo.h"

namespace jitcat::Reflection
{

	struct CustomMemberInfo: public TypeMemberInfo
	{
		CustomMemberInfo(const std::string& memberName, std::size_t memberOffset, const CatGenericType& type): TypeMemberInfo(memberName, type), memberOffset(memberOffset) {}
		virtual unsigned long long getOrdinal() const override final;

		std::size_t memberOffset;
	};


	template<typename BasicT>
	struct CustomBasicTypeMemberInfo: public CustomMemberInfo
	{
		CustomBasicTypeMemberInfo(const std::string& memberName, std::size_t memberOffset, const CatGenericType& type): CustomMemberInfo(memberName, memberOffset, type) {}

		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		void assign(std::any& base, const BasicT& valueToSet);
	};


	//Implements a TypeMemberInfo for class/struct pointer types that are reflectable.
	struct CustomTypeObjectMemberInfo: public CustomMemberInfo
	{
		CustomTypeObjectMemberInfo(const std::string& memberName, std::size_t memberOffset, const CatGenericType& type): CustomMemberInfo(memberName, memberOffset, type) {}

		virtual std::any getMemberReference(Reflectable* base) override final;
		virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		void assign(std::any& base, std::any& valueToSet);
	};


	//Implements a TypeMemberInfo for class/struct inline data types that are reflectable.
	struct CustomTypeObjectDataMemberInfo: public CustomMemberInfo
	{
		CustomTypeObjectDataMemberInfo(const std::string& memberName, std::size_t memberOffset, const CatGenericType& type): CustomMemberInfo(memberName, memberOffset, type) {}

		virtual std::any getMemberReference(Reflectable* base) override final;
		virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		void assign(std::any& base, std::any& valueToSet);
	};

}//End namespace jitcat::Reflection

#include "jitcat/CustomTypeMemberInfoHeaderImplementation.h"
