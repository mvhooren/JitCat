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
		CustomBasicTypeMemberInfo(const std::string& memberName, std::size_t memberOffset, const CatGenericType& type): TypeMemberInfo(memberName, type), memberOffset(memberOffset) {}

		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		std::size_t getMemberOffset() const;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		void assign(std::any& base, const T& valueToSet);

		std::size_t memberOffset;
	};


	//Implements a TypeMemberInfo for class/struct pointer types that are reflectable.
	struct CustomTypeObjectMemberInfo: public TypeMemberInfo
	{
		CustomTypeObjectMemberInfo(const std::string& memberName, std::size_t memberOffset, const CatGenericType& type): TypeMemberInfo(memberName, type), memberOffset(memberOffset) {}

		virtual std::any getMemberReference(Reflectable* base) override final;
		virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		std::size_t getMemberOffset() const;
		virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		void assign(std::any& base, std::any& valueToSet);
	
		std::size_t memberOffset;
	};


	//Implements a TypeMemberInfo for class/struct inline data types that are reflectable.
	struct CustomTypeObjectDataMemberInfo: public TypeMemberInfo
	{
		CustomTypeObjectDataMemberInfo(const std::string& memberName, std::size_t memberOffset, const CatGenericType& type): TypeMemberInfo(memberName, type), memberOffset(memberOffset) {}

		virtual std::any getMemberReference(Reflectable* base) override final;
		virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		std::size_t getMemberOffset() const;
		virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		void assign(std::any& base, std::any& valueToSet);
	
		std::size_t memberOffset;
	};

}//End namespace jitcat::Reflection

#include "jitcat/CustomTypeMemberInfoHeaderImplementation.h"
