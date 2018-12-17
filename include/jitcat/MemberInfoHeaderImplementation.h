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
inline llvm::Value* ContainerMemberInfo<T, U>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCodeGeneratorHelper* generatorHelper) const
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
	llvm::Value* memberOffset = generatorHelper->createIntPtrConstant(offset, "offsetTo_" + memberName);
	llvm::Value* parentObjectPointerInt = generatorHelper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
	llvm::Value* addressValue = generatorHelper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
	return generatorHelper->convertToPointer(addressValue, memberName + "_Ptr");
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
inline llvm::Value* ContainerMemberInfo<T, U>::generateIndex(std::map<std::string, ContainerItemType>* map, llvm::Value* containerPtr, llvm::Value* index, LLVMCodeGeneratorHelper* generatorHelper) const
{
	if (generatorHelper->isStringPointer(index))
	{
		static auto functionPointer = &ContainerMemberInfo<T, U>::getMapStringIndex<ContainerItemType>;
		return generatorHelper->createCall(LLVMTypes::functionRetPtrArgPtr_StringPtr, reinterpret_cast<uintptr_t>(functionPointer), {containerPtr, index}, "getMapStringIndex");
	}
	else
	{
		static auto functionPointer = &ContainerMemberInfo<T, U>::getMapIntIndex<ContainerItemType>;
		return generatorHelper->createCall(LLVMTypes::functionRetPtrArgPtr_Int, reinterpret_cast<uintptr_t>(functionPointer), {containerPtr, index}, "getMapIntIndex");
	}
}


template<typename T, typename U>
template<typename ContainerItemType>
inline llvm::Value* ContainerMemberInfo<T, U>::generateIndex(std::vector<ContainerItemType>* vector, llvm::Value* containerPtr, llvm::Value* index, LLVMCodeGeneratorHelper* generatorHelper) const
{
	static auto functionPointer = &ContainerMemberInfo<T, U>::getVectorIndex<ContainerItemType>;
	return generatorHelper->createCall(LLVMTypes::functionRetPtrArgPtr_Int, reinterpret_cast<uintptr_t>(functionPointer), {containerPtr, index}, "getVectorIndex");
}


template<typename T, typename U>
inline llvm::Value* ContainerMemberInfo<T, U>::generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVMCodeGeneratorHelper* generatorHelper) const
{
	//Index can either be an int or a string
	//container is a pointer to a vector or a map (of type T)
	U* nullContainer = nullptr;
	return generateIndex(nullContainer, container, index, generatorHelper);
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
inline llvm::Value* ClassPointerMemberInfo<T, U>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCodeGeneratorHelper* generatorHelper) const
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
	llvm::Value* memberOffset = generatorHelper->createIntPtrConstant(offset, "offsetTo_" + memberName);
	llvm::Value* parentObjectPointerInt = generatorHelper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
	llvm::Value* addressValue = generatorHelper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
	return generatorHelper->loadPointerAtAddress(addressValue, memberName);
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
inline llvm::Value* ClassObjectMemberInfo<T, U>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCodeGeneratorHelper* generatorHelper) const
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
	llvm::Value* memberOffset = generatorHelper->createIntPtrConstant(offset, "offsetTo_" + memberName);
	llvm::Value* parentObjectPointerInt = generatorHelper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
	llvm::Value* addressValue = generatorHelper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_Ptr");
	return generatorHelper->convertToPointer(addressValue, memberName);
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
inline llvm::Value* ClassUniquePtrMemberInfo<T, U>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCodeGeneratorHelper* generatorHelper) const
{
	llvm::Value* thisPointerAsInt = generatorHelper->createIntPtrConstant(reinterpret_cast<uintptr_t>(this), "ClassUniquePtrMemberInfoIntPtr");
	if (!generatorHelper->isPointer(parentObjectPointer))
	{
		parentObjectPointer = generatorHelper->convertToPointer(parentObjectPointer, memberName + "_Parent_Ptr");
	}
	llvm::Value* thisPointer = generatorHelper->convertToPointer(thisPointerAsInt, "ClassUniquePtrMemberInfoPtr");
	return generatorHelper->createCall(LLVMTypes::functionRetPtrArgPtr_Ptr, reinterpret_cast<uintptr_t>(&ClassUniquePtrMemberInfo<T,U>::getPointer), {parentObjectPointer, thisPointer}, "getUniquePtr");
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
inline llvm::Value* BasicTypeMemberInfo<T, U>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCodeGeneratorHelper* generatorHelper) const
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
	
	llvm::Value* memberOffset = generatorHelper->createIntPtrConstant(offset, "offsetTo_" + memberName);
	llvm::Value* parentObjectPointerInt = generatorHelper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
	llvm::Value* addressValue = generatorHelper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
	if constexpr (std::is_same<U, std::string>::value)
	{
		//std::string case (returns a pointer to the std::string)
		return generatorHelper->convertToPointer(addressValue, memberName, LLVMTypes::stringPtrType);
	}
	else
	{
		//int, bool, float case	(returns by value)
		return generatorHelper->loadBasicType(generatorHelper->toLLVMType(catType), addressValue, memberName);
	}
}