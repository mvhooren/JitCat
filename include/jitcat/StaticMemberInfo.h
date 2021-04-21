/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/MemberVisibility.h"

#include <any>
#include <map>
#include <vector>


namespace jitcat::LLVM
{
	class LLVMCodeGeneratorHelper;
	struct LLVMCompileTimeContext;
}


namespace jitcat::Reflection
{
	class ReflectableHandle;

	struct StaticMemberInfo
	{
		StaticMemberInfo(): visibility(MemberVisibility::Private) {}
		StaticMemberInfo(const std::string& memberName, const CatGenericType& type): catType(type), visibility(MemberVisibility::Public), memberName(memberName)  {}
		virtual ~StaticMemberInfo() {};
		virtual std::any getMemberReference();
		virtual std::any getAssignableMemberReference();
		virtual llvm::Value* generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const;
		virtual llvm::Value* generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const;
		virtual llvm::Value* generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const;

		CatGenericType catType;
		MemberVisibility visibility;

		std::string memberName;
	};


	//Implements a StaticMemberInfo for class/struct pointer types that are reflectable.
	struct StaticClassPointerMemberInfo: public StaticMemberInfo
	{
		StaticClassPointerMemberInfo(const std::string& memberName, unsigned char** memberPointer, const CatGenericType& type);

		virtual std::any getMemberReference() override final;
		virtual std::any getAssignableMemberReference() override final;
		virtual llvm::Value* generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const override final;
		virtual llvm::Value* generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		unsigned char** memberPointer;
	};


	//Implements a StaticMemberInfo for class/struct pointer types that are reflectable.
	struct StaticClassHandleMemberInfo: public StaticMemberInfo
	{
		StaticClassHandleMemberInfo(const std::string& memberName, ReflectableHandle* memberPointer, const CatGenericType& type): StaticMemberInfo(memberName, type), memberPointer(memberPointer) {}

		virtual std::any getMemberReference() override final;
		virtual std::any getAssignableMemberReference() override final;
		virtual llvm::Value* generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const override final;
		virtual llvm::Value* generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		ReflectableHandle* memberPointer;
	};

	//Implements a StaticMemberInfo for class/struct types that are reflectable.
	struct StaticClassObjectMemberInfo: public StaticMemberInfo
	{
		StaticClassObjectMemberInfo(const std::string& memberName, unsigned char* memberPointer, const CatGenericType& type): StaticMemberInfo(memberName, type), memberPointer(memberPointer) {}

		virtual std::any getMemberReference() override final;
		virtual std::any getAssignableMemberReference() override final;

		virtual llvm::Value* generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const override final;

		unsigned char* memberPointer;
	};


	//Implements a StaticMemberInfo for a unique_ptr to class/struct types that are reflectable.
	template<typename ClassT>
	struct StaticClassUniquePtrMemberInfo: public StaticMemberInfo
	{
		StaticClassUniquePtrMemberInfo(const std::string& memberName, std::unique_ptr<ClassT>* memberPointer, const CatGenericType& type);
		static ClassT* getPointer(std::unique_ptr<ClassT>* info);
		inline virtual std::any getMemberReference() override final;
		inline virtual std::any getAssignableMemberReference() override final;
		inline virtual llvm::Value* generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const override final;
	private:
		inline std::string getMangledGetPointerName() const;

	private:
		std::unique_ptr<ClassT>* memberPointer;
	};


	//Implements a StaticMemberInfo for basic types.
	template<typename BasicT>
	struct StaticBasicTypeMemberInfo: public StaticMemberInfo
	{
		StaticBasicTypeMemberInfo(const std::string& memberName, BasicT* memberPointer, const CatGenericType& type): StaticMemberInfo(memberName, type), memberPointer(memberPointer) {}
	
		inline virtual std::any getMemberReference() override final;
		inline virtual std::any getAssignableMemberReference() override final;

		inline virtual llvm::Value* generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		BasicT* memberPointer;
	};
}

#include "jitcat/StaticMemberInfoHeaderImplementation.h"
