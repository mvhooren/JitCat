/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "BasicTypeMemberReference.h"
#include "ContainerMemberReference.h"
#include "ObjectMemberReference.h"
#include "LLVMCodeGeneratorHelper.h"
#include "LLVMCompileTimeContext.h"
#include "LLVMTypes.h"
#include "MemberInfo.h"
#include "Tools.h"


template<typename T, typename U>
inline MemberReferencePtr ContainerMemberInfo<T, U>::getMemberReference(MemberReferencePtr& base)
{
	if (!base.isNull())
	{
		T* baseObject = static_cast<T*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			U& container = baseObject->*memberPointer;
			return new ContainerMemberReference<U>(container, this, baseObject, nestedType);
		}
	}
	return nullptr;
}


template<typename T, typename U>
inline llvm::Value* ContainerMemberInfo<T, U>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCompileTimeContext* context) const
{
	static_assert(sizeof(memberPointer) == 4 || sizeof(memberPointer) == 8, "Expected a 4 or 8 byte member pointer. Object may use virtual inheritance which is not supported.");
	unsigned long long offset = 0;
	if constexpr (sizeof(memberPointer) == 4)
	{
		unsigned int smallOffset = 0;
		memcpy(&smallOffset, &memberPointer, 4);
		offset = smallOffset;
	}
	else
	{
		memcpy(&offset, &memberPointer, 8);
	}
	auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
	{
		llvm::Value* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
		return context->helper->convertToPointer(addressValue, memberName + "_Ptr");
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVMTypes::pointerType, context);
}


template<typename T, typename U>
template<typename ContainerItemType>
inline ContainerItemType ContainerMemberInfo<T, U>::getMapIntIndex(std::map<std::string, ContainerItemType>* map, int index)
{
	int count = 0;
	for (auto& iter : (*map))
	{
		if (count == index)
		{
			return iter.second;
		}
		count++;
	}
	return nullptr;
}


template<typename T, typename U>
template<typename ContainerItemType>
inline ContainerItemType ContainerMemberInfo<T, U>::getMapStringIndex(std::map<std::string, ContainerItemType>* map, std::string* index)
{
	std::string lowerCaseIdx = Tools::toLowerCase(*index);
	auto iter = map->find(lowerCaseIdx);
	if (iter != map->end())
	{
		return iter->second;
	}
	return nullptr;
}


template<typename T, typename U>
template<typename ContainerItemType>
inline ContainerItemType ContainerMemberInfo<T, U>::getVectorIndex(std::vector<ContainerItemType>* vector, int index)
{
	if (index >= 0 && index < vector->size())
	{
		return vector->operator[](index);
	}
	return nullptr;
}


template<typename T, typename U>
template<typename ContainerItemType>
inline llvm::Value* ContainerMemberInfo<T, U>::generateIndex(std::map<std::string, ContainerItemType>* map, llvm::Value* containerPtr, llvm::Value* index, LLVMCompileTimeContext* context) const
{
	if (context->helper->isStringPointer(index))
	{
		auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
		{
			static auto functionPointer = &ContainerMemberInfo<T, U>::getMapStringIndex<ContainerItemType>;
			return compileContext->helper->createCall(LLVMTypes::functionRetPtrArgPtr_StringPtr, reinterpret_cast<uintptr_t>(functionPointer), {containerPtr, index}, "getMapStringIndex");
		};
		return context->helper->createOptionalNullCheckSelect(containerPtr, notNullCodeGen, LLVMTypes::getLLVMType<ContainerItemType>(), context);
	}
	else
	{
		auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
		{
			static auto functionPointer = &ContainerMemberInfo<T, U>::getMapIntIndex<ContainerItemType>;
			return compileContext->helper->createCall(LLVMTypes::functionRetPtrArgPtr_Int, reinterpret_cast<uintptr_t>(functionPointer), {containerPtr, index}, "getMapIntIndex");
		};
		return context->helper->createOptionalNullCheckSelect(containerPtr, notNullCodeGen, LLVMTypes::getLLVMType<ContainerItemType>(), context);
	}
}


template<typename T, typename U>
template<typename ContainerItemType>
inline llvm::Value* ContainerMemberInfo<T, U>::generateIndex(std::vector<ContainerItemType>* vector, llvm::Value* containerPtr, llvm::Value* index, LLVMCompileTimeContext* context) const
{
	auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
	{
		static auto functionPointer = &ContainerMemberInfo<T, U>::getVectorIndex<ContainerItemType>;
		return compileContext->helper->createCall(LLVMTypes::functionRetPtrArgPtr_Int, reinterpret_cast<uintptr_t>(functionPointer), {containerPtr, index}, "getVectorIndex");
	};
	return context->helper->createOptionalNullCheckSelect(containerPtr, notNullCodeGen, LLVMTypes::getLLVMType<ContainerItemType>(), context);
}


template<typename T, typename U>
inline llvm::Value* ContainerMemberInfo<T, U>::generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVMCompileTimeContext* context) const
{
	//Index can either be an int or a string
	//container is a pointer to a vector or a map (of type T)
	U* nullContainer = nullptr;
	return generateIndex(nullContainer, container, index, context);
}


template<typename T, typename U>
inline MemberReferencePtr ClassPointerMemberInfo<T, U>::getMemberReference(MemberReferencePtr& base)
{
	if (!base.isNull())
	{
		T* baseObject = static_cast<T*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			U* member = baseObject->*memberPointer;
			return new ObjectMemberReference<U>(member, this, nestedType);
		}
	}
	return nullptr;
}


template<typename T, typename U>
inline llvm::Value* ClassPointerMemberInfo<T, U>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCompileTimeContext* context) const
{
	static_assert(sizeof(memberPointer) == 4 || sizeof(memberPointer) == 8, "Expected a 4 or 8 byte member pointer. Object may use virtual inheritance which is not supported.");
	unsigned long long offset = 0;
	if constexpr (sizeof(memberPointer) == 4)
	{
		unsigned int smallOffset = 0;
		memcpy(&smallOffset, &memberPointer, 4);
		offset = smallOffset;
	}
	else
	{
		memcpy(&offset, &memberPointer, 8);
	}
	auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
	{
		llvm::Value* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
		return context->helper->loadPointerAtAddress(addressValue, memberName);
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVMTypes::pointerType, context);
}


template<typename T, typename U>
inline MemberReferencePtr ClassObjectMemberInfo<T, U>::getMemberReference(MemberReferencePtr & base)
{
	if (!base.isNull())
	{
		T* baseObject = static_cast<T*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			return new ObjectMemberReference<U>(&(baseObject->*memberPointer), this, nestedType);
		}
	}
	return nullptr;
}


template<typename T, typename U>
inline llvm::Value* ClassObjectMemberInfo<T, U>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCompileTimeContext* context) const
{
	static_assert(sizeof(memberPointer) == 4 || sizeof(memberPointer) == 8, "Expected a 4 or 8 byte member pointer. Object may use virtual inheritance which is not supported.");
	unsigned long long offset = 0;
	if constexpr (sizeof(memberPointer) == 4)
	{
		unsigned int smallOffset = 0;
		memcpy(&smallOffset, &memberPointer, 4);
		offset = smallOffset;
	}
	else
	{
		memcpy(&offset, &memberPointer, 8);
	}
	auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
	{
		llvm::Value* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_Ptr");
		return context->helper->convertToPointer(addressValue, memberName);
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVMTypes::pointerType, context);
}


template<typename T, typename U>
inline U* ClassUniquePtrMemberInfo<T, U>::getPointer(T* parentObject, ClassUniquePtrMemberInfo<T, U>* info)
 {
	std::unique_ptr<U> T::* memberPointer = info->memberPointer;
	return (parentObject->*memberPointer).get();
}


template<typename T, typename U>
inline MemberReferencePtr ClassUniquePtrMemberInfo<T, U>::getMemberReference(MemberReferencePtr& base)
{
	if (!base.isNull())
	{
		T* baseObject = static_cast<T*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			return new ObjectMemberReference<U>((baseObject->*memberPointer).get(), this, nestedType);
		}
	}
	return nullptr;
}


template<typename T, typename U>
inline llvm::Value* ClassUniquePtrMemberInfo<T, U>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCompileTimeContext* context) const
{
	llvm::Value* thisPointerAsInt = context->helper->createIntPtrConstant(reinterpret_cast<uintptr_t>(this), "ClassUniquePtrMemberInfoIntPtr");
	if (!context->helper->isPointer(parentObjectPointer))
	{
		parentObjectPointer = context->helper->convertToPointer(parentObjectPointer, memberName + "_Parent_Ptr");
	}
	auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
	{
		llvm::Value* thisPointer = context->helper->convertToPointer(thisPointerAsInt, "ClassUniquePtrMemberInfoPtr");
		return context->helper->createCall(LLVMTypes::functionRetPtrArgPtr_Ptr, reinterpret_cast<uintptr_t>(&ClassUniquePtrMemberInfo<T,U>::getPointer), {parentObjectPointer, thisPointer}, "getUniquePtr");
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVMTypes::pointerType, context);
}


template<typename T, typename U>
inline MemberReferencePtr BasicTypeMemberInfo<T, U>::getMemberReference(MemberReferencePtr& base)
{
	if (!base.isNull())
	{
		T* objectPointer = static_cast<T*>(base->getParentObject());
		if (objectPointer != nullptr)
		{
			U& value = objectPointer->*memberPointer;

			return new BasicTypeMemberReference<U>(value, this, objectPointer, isWritable);
		}
	}
	return nullptr;
}


template<typename T, typename U>
inline llvm::Value* BasicTypeMemberInfo<T, U>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCompileTimeContext* context) const
{
	static_assert(sizeof(memberPointer) == 4 || sizeof(memberPointer) == 8, "Expected a 4 or 8 byte member pointer. Object may use virtual inheritance which is not supported.");
	unsigned long long offset = 0;
	if constexpr (sizeof(memberPointer) == 4)
	{
		unsigned int smallOffset = 0;
		memcpy(&smallOffset, &memberPointer, 4);
		offset = smallOffset;
	}
	else
	{
		memcpy(&offset, &memberPointer, 8);
	}
	auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
	{	
		llvm::Value* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
		if constexpr (std::is_same<U, std::string>::value)
		{
			//std::string case (returns a pointer to the std::string)
			return context->helper->convertToPointer(addressValue, memberName, LLVMTypes::stringPtrType);
		}
		else
		{
			//int, bool, float case	(returns by value)
			return context->helper->loadBasicType(context->helper->toLLVMType(catType), addressValue, memberName);
		}
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, context->helper->toLLVMType(catType), context);
}