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
#include "jitcat/AssignableType.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/ContainerType.h"
#include "jitcat/LLVMForwardDeclares.h"
#include "jitcat/MemberFlags.h"
#include "jitcat/TypeRegistry.h"

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
		TypeMemberInfo() {}
		TypeMemberInfo(const std::string& memberName, const CatGenericType& type): memberName(memberName), catType(type) {}
		virtual ~TypeMemberInfo() {};
		inline virtual std::any getMemberReference(Reflectable* base) { return nullptr; }
		inline virtual std::any getAssignableMemberReference(Reflectable* base, AssignableType& assignableType) { return nullptr; }
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const {return nullptr;};
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const {return nullptr;};
		inline virtual llvm::Value* generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const {return nullptr;};

		CatGenericType catType;

		std::string memberName;
	};


	//Implements a TypeMemberInfo for container types.
	template<typename T, typename U>
	struct ContainerMemberInfo: public TypeMemberInfo
	{
		ContainerMemberInfo(const std::string& memberName, U T::* memberPointer, const CatGenericType& type): TypeMemberInfo(memberName, type), memberPointer(memberPointer) {}

		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base, AssignableType& assignableType) override final;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
	
		template<typename ContainerItemType, typename Compare>
		static Reflection::Reflectable* getMapIntIndex(std::map<std::string, ContainerItemType, Compare>* map, int index);
		template<typename ContainerItemType, typename Compare>
		static Reflection::Reflectable* getMapStringIndex(std::map<std::string, ContainerItemType, Compare>* map, std::string* index);
		template<typename ContainerItemType>
		static Reflection::Reflectable* getVectorIndex(std::vector<ContainerItemType>* vector, int index);

		template<typename ContainerItemType, typename Compare>
		inline llvm::Value* generateIndex(std::map<std::string, ContainerItemType, Compare>* map, llvm::Value* containerPtr, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const;
		template<typename ContainerItemType>
		inline llvm::Value* generateIndex(std::vector<ContainerItemType>* vector, llvm::Value* containerPtr, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const;
		inline virtual llvm::Value* generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const override final;


		U T::* memberPointer;
	};


	//Implements a TypeMemberInfo for class/struct pointer types that are reflectable.
	template<typename T, typename U>
	struct ClassPointerMemberInfo: public TypeMemberInfo
	{
		ClassPointerMemberInfo(const std::string& memberName, U* T::* memberPointer, const CatGenericType& type): TypeMemberInfo(memberName, type), memberPointer(memberPointer) {}

		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base, AssignableType& assignableType) override final;
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
		inline virtual std::any getAssignableMemberReference(Reflectable* base, AssignableType& assignableType) override final;

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
		inline virtual std::any getAssignableMemberReference(Reflectable* base, AssignableType& assignableType) override final;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;

		std::unique_ptr<U> T::* memberPointer;
	};



	//Implements a TypeMemberInfo for basic types.
	template<typename T, typename U>
	struct BasicTypeMemberInfo: public TypeMemberInfo
	{
		BasicTypeMemberInfo(const std::string& memberName, U T::* memberPointer, const CatGenericType& type): TypeMemberInfo(memberName, type), memberPointer(memberPointer) {}
	
		inline virtual std::any getMemberReference(Reflectable* base) override final;
		inline virtual std::any getAssignableMemberReference(Reflectable* base, AssignableType& assignableType) override final;

		unsigned long long getMemberPointerOffset() const;
		inline virtual llvm::Value* generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const override final;
		inline virtual llvm::Value* generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const override final;


		U T::* memberPointer;
	};

} //End namespace jitcat::Reflection

#include "jitcat/MemberInfoHeaderImplementation.h"



