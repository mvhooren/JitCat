/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::LLVM
{
	class LLVMCodeGeneratorHelper;
	struct LLVMCompileTimeContext;
}

#include "jitcat/CatGenericType.h"
#include "jitcat/ContainerType.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/MemberFlags.h"
#include "jitcat/MemberVisibility.h"
#include "jitcat/TypeRegistry.h"
#include "jitcat/TypeTraits.h"

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
	struct TypeMemberInfo
	{
		TypeMemberInfo(): visibility(MemberVisibility::Private) {}
		TypeMemberInfo(const std::string& memberName, const CatGenericType& type): memberName(memberName), catType(type), visibility(MemberVisibility::Public) {}
		virtual ~TypeMemberInfo() {};
		inline virtual std::any getMemberReference(Reflectable* base);
		inline virtual std::any getAssignableMemberReference(Reflectable* base);
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const;
		inline virtual llvm::Value* generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const;
		inline virtual bool isDeferred() const { return false; }
		TypeMemberInfo* toDeferredTypeMemberInfo(TypeMemberInfo* baseMember);

		CatGenericType catType;
		MemberVisibility visibility;

		std::string memberName;
	};

	struct DeferredMemberInfo: public TypeMemberInfo
	{
		DeferredMemberInfo(TypeMemberInfo* baseMember, TypeMemberInfo* deferredMember):
			TypeMemberInfo(deferredMember->memberName, deferredMember->catType),
			baseMember(baseMember),
			deferredMember(deferredMember)
		{}

		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual bool isDeferred() const override final { return true; }
		TypeMemberInfo* baseMember;
		TypeMemberInfo* deferredMember;
	};


	//Implements a TypeMemberInfo for container types.
	template<typename T, typename U>
	struct ContainerMemberInfo: public TypeMemberInfo
	{
		ContainerMemberInfo(const std::string& memberName, U T::* memberPointer, const CatGenericType& type): TypeMemberInfo(memberName, type), memberPointer(memberPointer) {}

		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
	
		template<typename ContainerKeyType, typename ContainerItemType, typename CompareT, typename AllocatorT>
		static typename TypeTraits<ContainerItemType>::functionReturnType getMapIntIndex(std::map<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>* map, int index);

		template<typename ContainerKeyType, typename ContainerItemType, typename CompareT, typename AllocatorT>
		static typename TypeTraits<ContainerItemType>::functionReturnType  getMapKeyIndex(std::map<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>* map, typename TypeTraits<ContainerKeyType>::functionParameterType index);

		template<typename ContainerItemType, typename AllocatorT>
		static typename TypeTraits<ContainerItemType>::functionReturnType getVectorIndex(std::vector<ContainerItemType, AllocatorT>* vector, int index);

		template<typename ContainerKeyType, typename ContainerItemType, typename CompareT, typename AllocatorT>
		inline llvm::Value* generateIndex(std::map<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>* map, llvm::Value* containerPtr, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const;

		template<typename ContainerItemType, typename AllocatorT>
		inline llvm::Value* generateIndex(std::vector<ContainerItemType, AllocatorT>* vector, llvm::Value* containerPtr, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const;

		inline virtual llvm::Value* generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const override final;


		U T::* memberPointer;
	};


	//Implements a TypeMemberInfo for class/struct pointer types that are reflectable.
	template<typename T, typename U>
	struct ClassPointerMemberInfo: public TypeMemberInfo
	{
		ClassPointerMemberInfo(const std::string& memberName, U* T::* memberPointer, const CatGenericType& type): TypeMemberInfo(memberName, type), memberPointer(memberPointer) {}

		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		unsigned long long getMemberPointerOffset() const;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;

		U* T::* memberPointer;
	};

	//Implements a TypeMemberInfo for class/struct types that are reflectable.
	template<typename T, typename U>
	struct ClassObjectMemberInfo: public TypeMemberInfo
	{
		ClassObjectMemberInfo(const std::string& memberName, U T::* memberPointer, const CatGenericType& type): TypeMemberInfo(memberName, type), memberPointer(memberPointer) {}

		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base) override final;

		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;

		U T::* memberPointer;
	};


	//Implements a TypeMemberInfo for a unique_ptr to class/struct types that are reflectable.
	template<typename T, typename U>
	struct ClassUniquePtrMemberInfo: public TypeMemberInfo
	{
		ClassUniquePtrMemberInfo(const std::string& memberName, std::unique_ptr<U> T::* memberPointer, const CatGenericType& type): TypeMemberInfo(memberName, type), memberPointer(memberPointer) {}
		static U* getPointer(T* parentObject, ClassUniquePtrMemberInfo<T, U>* info);
		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base) override final;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;

		std::unique_ptr<U> T::* memberPointer;
	};



	//Implements a TypeMemberInfo for basic types.
	template<typename T, typename U>
	struct BasicTypeMemberInfo: public TypeMemberInfo
	{
		BasicTypeMemberInfo(const std::string& memberName, U T::* memberPointer, const CatGenericType& type): TypeMemberInfo(memberName, type), memberPointer(memberPointer) {}
	
		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base) override final;

		unsigned long long getMemberPointerOffset() const;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;


		U T::* memberPointer;
	};

} //End namespace jitcat::Reflection

#include "jitcat/MemberInfoHeaderImplementation.h"



