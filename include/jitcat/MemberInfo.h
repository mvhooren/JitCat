/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/MemberFlags.h"
#include "jitcat/TypeMemberInfo.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace jitcat::Reflection
{
	class Reflectable;
	class TypeInfo;


	//This struct contains type information on a single member of a reflectable object
	//A member can be:
	//- A basic type (int, float, bool, std::string)
	//- A container type, vector<T*> or map<std::string, T*>, where T is a reflectable type
	//- A nested reflectable type pointer

	//See TypeInfo.h for more information

	struct DeferredMemberInfo: public TypeMemberInfo
	{
		DeferredMemberInfo(TypeMemberInfo* baseMember, TypeMemberInfo* deferredMember):
			TypeMemberInfo(deferredMember->getMemberName(), deferredMember->getType()),
			baseMember(baseMember),
			deferredMember(deferredMember)
		{}

		virtual std::any getMemberReference(unsigned char* base) override final;
		virtual std::any getAssignableMemberReference(unsigned char* base) override final;
		virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;
		virtual llvm::Value* generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const override final;
		virtual bool isDeferred() const override final { return true; }
		virtual unsigned long long getOrdinal() const override final;

	private:
		TypeMemberInfo* baseMember;
		TypeMemberInfo* deferredMember;
	};


	//Implements a TypeMemberInfo for class/struct pointer types that are reflectable.
	template<typename BaseT, typename ClassT>
	struct ClassPointerMemberInfo: public TypeMemberInfo
	{
		ClassPointerMemberInfo(const std::string& memberName, ClassT* BaseT::* memberPointer, const CatGenericType& type);

		inline virtual std::any getMemberReference(unsigned char* base) override final;
		inline virtual std::any getAssignableMemberReference(unsigned char* base) override final;
		unsigned long long getMemberPointerOffset() const;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		inline virtual unsigned long long getOrdinal() const override final;
	
	private:
		inline std::string getMemberOffsetName() const;

	private:
		ClassT* BaseT::* memberPointer;
	};

	//Implements a TypeMemberInfo for class/struct types that are reflectable.
	template<typename BaseT, typename ClassT>
	struct ClassObjectMemberInfo: public TypeMemberInfo
	{
		ClassObjectMemberInfo(const std::string& memberName, ClassT BaseT::* memberPointer, const CatGenericType& type);

		inline virtual std::any getMemberReference(unsigned char* base) override final;
		inline virtual std::any getAssignableMemberReference(unsigned char* base) override final;

		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;

		inline virtual unsigned long long getOrdinal() const override final;
	
	private:
		inline std::string getMemberOffsetName() const;

	private:
		ClassT BaseT::* memberPointer;
	};


	//Implements a TypeMemberInfo for a unique_ptr to class/struct types that are reflectable.
	template<typename BaseT, typename ClassT>
	struct ClassUniquePtrMemberInfo: public TypeMemberInfo
	{
		ClassUniquePtrMemberInfo(const std::string& memberName, std::unique_ptr<ClassT> BaseT::* memberPointer, const CatGenericType& type);
		static ClassT* getPointer(BaseT* parentObject, ClassUniquePtrMemberInfo<BaseT, ClassT>* info);
		inline virtual std::any getMemberReference(unsigned char* base) override final;
		inline virtual std::any getAssignableMemberReference(unsigned char* base) override final;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;

		inline virtual unsigned long long getOrdinal() const override final;
	private:
		inline std::string getMangledGetPointerName() const;
		inline std::string getGlobalThisVariableName() const;

	private:
		std::unique_ptr<ClassT> BaseT::* memberPointer;
	};



	//Implements a TypeMemberInfo for basic types.
	template<typename BaseT, typename BasicT>
	struct BasicTypeMemberInfo: public TypeMemberInfo
	{
		BasicTypeMemberInfo(const std::string& memberName, BasicT BaseT::* memberPointer, const CatGenericType& type);
	
		inline virtual std::any getMemberReference(unsigned char* base) override final;
		inline virtual std::any getAssignableMemberReference(unsigned char* base) override final;

		unsigned long long getMemberPointerOffset() const;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual unsigned long long getOrdinal() const override final;

	private:
		inline std::string getMemberOffsetName() const;

	private:
		BasicT BaseT::* memberPointer;
	};

} //End namespace jitcat::Reflection

#include "jitcat/MemberInfoHeaderImplementation.h"



