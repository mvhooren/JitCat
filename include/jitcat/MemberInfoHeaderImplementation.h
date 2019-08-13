/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/Tools.h"

namespace jitcat::Reflection
{


template<typename BaseT, typename ContainerT>
inline std::any ContainerMemberInfo<BaseT, ContainerT>::getMemberReference(Reflectable* base)
{
	BaseT* baseObject = static_cast<BaseT*>(base);
	if (baseObject != nullptr)
	{
		ContainerT& container = baseObject->*memberPointer;
		return &container;
	}
	return (ContainerT*)nullptr;
}


template<typename BaseT, typename ContainerT>
inline std::any ContainerMemberInfo<BaseT, ContainerT>::getAssignableMemberReference(Reflectable* base)
{
	return getMemberReference(base);
}


template<typename BaseT, typename ContainerT>
inline llvm::Value* ContainerMemberInfo<BaseT, ContainerT>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
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
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		llvm::Value* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
		return context->helper->convertToPointer(addressValue, memberName + "_Ptr");
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVM::LLVMTypes::pointerType, context);
#else 
	return nullptr;
#endif // ENABLE_LLVM
}


template<typename BaseT, typename ContainerT>
template<typename ContainerKeyType, typename ContainerItemType, typename CompareT, typename AllocatorT>
inline typename TypeTraits<ContainerItemType>::functionReturnType ContainerMemberInfo<BaseT, ContainerT>::getMapIntIndex(std::map<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>* map, int index)
{
	int count = 0;
	for (auto& iter : (*map))
	{
		if (count == index)
		{
			return TypeTraits<ContainerItemType>::stripValue(iter.second);
		}
		count++;
	}
	return TypeTraits<ContainerItemType>::getDefaultValue();
}


template<typename BaseT, typename ContainerT>
template<typename ContainerKeyType, typename ContainerItemType, typename CompareT, typename AllocatorT>
inline typename TypeTraits<ContainerItemType>::functionReturnType ContainerMemberInfo<BaseT, ContainerT>::getMapKeyIndex(std::map<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>* map, typename TypeTraits<ContainerKeyType>::functionParameterType index)
{
	if constexpr (std::is_same<ContainerKeyType, std::string>::value)
	{
		if (&index != nullptr)
		{
			std::string lowerCaseIdx = Tools::toLowerCase(index);
			auto iter = map->find(lowerCaseIdx);
			if (iter != map->end())
			{
				return TypeTraits<ContainerItemType>::stripValue(iter->second);
			}
		}
	}
	else
	{
		auto iter = map->find(index);
		if (iter != map->end())
		{
			return TypeTraits<ContainerItemType>::stripValue(iter->second);
		}
	}
	return TypeTraits<ContainerItemType>::getDefaultValue();
}


template<typename BaseT, typename ContainerT>
template<typename ContainerItemType, typename AllocatorT>
inline typename TypeTraits<ContainerItemType>::functionReturnType ContainerMemberInfo<BaseT, ContainerT>::getVectorIndex(std::vector<ContainerItemType, AllocatorT>* vector, int index)
{
	if (index >= 0 && index < (int)vector->size())
	{
		return TypeTraits<ContainerItemType>::stripValue(vector->operator[](index));
	}
	return TypeTraits<ContainerItemType>::getDefaultValue();
}


template<typename BaseT, typename ContainerT>
template<typename ContainerKeyType, typename ContainerItemType, typename CompareT, typename AllocatorT>
inline llvm::Value* ContainerMemberInfo<BaseT, ContainerT>::generateIndex(std::map<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>* map, llvm::Value* containerPtr, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	if (!context->helper->isInt(index) || std::is_same<int, ContainerKeyType>::value)
	{
		auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
		{
			static auto functionPointer = &ContainerMemberInfo<BaseT, ContainerT>::getMapKeyIndex<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>;
			return compileContext->helper->createCall(context, functionPointer, {containerPtr, index}, "getMapKeyIndex");
		};
		return context->helper->createOptionalNullCheckSelect(containerPtr, notNullCodeGen, LLVM::LLVMTypes::getLLVMType<ContainerItemType>(), context);
	}
	else
	{
		auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
		{
			static auto functionPointer = &ContainerMemberInfo<BaseT, ContainerT>::getMapIntIndex<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>;
			return compileContext->helper->createCall(context, functionPointer, {containerPtr, index}, "getMapIntIndex");
		};
		return context->helper->createOptionalNullCheckSelect(containerPtr, notNullCodeGen, LLVM::LLVMTypes::getLLVMType<ContainerItemType>(), context);
	}
#else
	return nullptr;
#endif //ENABLE_LLVM
}


template<typename BaseT, typename ContainerT>
template<typename ContainerItemType, typename AllocatorT>
inline llvm::Value* ContainerMemberInfo<BaseT, ContainerT>::generateIndex(std::vector<ContainerItemType, AllocatorT>* vector, llvm::Value* containerPtr, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		static auto functionPointer = &ContainerMemberInfo<BaseT, ContainerT>::getVectorIndex<ContainerItemType, AllocatorT>;
		return compileContext->helper->createCall(context, functionPointer, {containerPtr, index}, "getVectorIndex");
	};
	return context->helper->createOptionalNullCheckSelect(containerPtr, notNullCodeGen, LLVM::LLVMTypes::getLLVMType<ContainerItemType>(), context);
#else
	return nullptr;
#endif //ENABLE_LLVM
}


template<typename BaseT, typename ContainerT>
inline llvm::Value* ContainerMemberInfo<BaseT, ContainerT>::generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
{
	//Index can either be an int or a string
	//container is a pointer to a vector or a map (of type ContainerT)
	ContainerT* nullContainer = nullptr;
	return generateIndex(nullContainer, container, index, context);
}


template<typename BaseT, typename ClassT>
inline std::any ClassPointerMemberInfo<BaseT, ClassT>::getMemberReference(Reflectable* base)
{
	BaseT* baseObject = static_cast<BaseT*>(base);
	if (baseObject != nullptr)
	{
		ClassT* member = baseObject->*memberPointer;
		return static_cast<Reflectable*>(member);
	}
	return static_cast<Reflectable*>(nullptr);
}

template<typename BaseT, typename ClassT>
inline std::any ClassPointerMemberInfo<BaseT, ClassT>::getAssignableMemberReference(Reflectable* base)
{
	BaseT* baseObject = static_cast<BaseT*>(base);
	if (baseObject != nullptr)
	{
		ClassT** member = &(baseObject->*memberPointer);
		return reinterpret_cast<Reflectable**>(member);
	}
	return static_cast<Reflectable**>(nullptr);
}


template<typename BaseT, typename ClassT>
inline unsigned long long ClassPointerMemberInfo<BaseT, ClassT>::getMemberPointerOffset() const
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
	return offset;
}


template<typename BaseT, typename ClassT>
inline llvm::Value* ClassPointerMemberInfo<BaseT, ClassT>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	unsigned long long offset = getMemberPointerOffset();
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		llvm::Value* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
		return context->helper->loadPointerAtAddress(addressValue, memberName);
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVM::LLVMTypes::pointerType, context);
#else 
	return nullptr;
#endif // ENABLE_LLVM
}


template<typename BaseT, typename ClassT>
inline llvm::Value* ClassPointerMemberInfo<BaseT, ClassT>::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	unsigned long long offset = getMemberPointerOffset();
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		llvm::Value* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressIntValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
		llvm::Value* addressValue = context->helper->convertToPointer(addressIntValue, memberName + "_Ptr", context->helper->toLLVMPtrType(catType));
		context->helper->writeToPointer(addressValue, rValue);
		return rValue;
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVM::LLVMTypes::pointerType, context);
#else
	return nullptr;
#endif // ENABLE_LLVM
}


template<typename BaseT, typename ClassT>
inline std::any ClassObjectMemberInfo<BaseT, ClassT>::getMemberReference(Reflectable* base)
{
	BaseT* baseObject = static_cast<BaseT*>(base);
	if (baseObject != nullptr)
	{
		return static_cast<Reflectable*>(&(baseObject->*memberPointer));
	}
	return static_cast<Reflectable*>(nullptr);
}


template<typename BaseT, typename ClassT>
inline std::any ClassObjectMemberInfo<BaseT, ClassT>::getAssignableMemberReference(Reflectable* base)
{
	//Not supported for now (would require implementing calling of operator= on target object)
	return getMemberReference(base);
}


template<typename BaseT, typename ClassT>
inline llvm::Value* ClassObjectMemberInfo<BaseT, ClassT>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
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
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		llvm::Value* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_Ptr");
		return context->helper->convertToPointer(addressValue, memberName);
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVM::LLVMTypes::pointerType, context);
#else 
	return nullptr;
#endif // ENABLE_LLVM
}


template<typename BaseT, typename ClassT>
inline ClassT* ClassUniquePtrMemberInfo<BaseT, ClassT>::getPointer(BaseT* parentObject, ClassUniquePtrMemberInfo<BaseT, ClassT>* info)
 {
	std::unique_ptr<ClassT> BaseT::* memberPointer = info->memberPointer;
	return (parentObject->*memberPointer).get();
}


template<typename BaseT, typename ClassT>
inline std::any ClassUniquePtrMemberInfo<BaseT, ClassT>::getMemberReference(Reflectable* base)
{
	BaseT* baseObject = static_cast<BaseT*>(base);
	if (baseObject != nullptr)
	{
		return static_cast<Reflectable*>((baseObject->*memberPointer).get());
	}
	return static_cast<Reflectable*>(nullptr);
}


template<typename BaseT, typename ClassT>
inline std::any ClassUniquePtrMemberInfo<BaseT, ClassT>::getAssignableMemberReference(Reflectable* base)
{
	//Cannot assing unique_ptr, this would transfer ownership and potentially delete the pointer at some point. Bad idea.
	//The pointer may for example have come from another unique_ptr.
	return std::any((ClassT*)nullptr);
}


template<typename BaseT, typename ClassT>
inline llvm::Value* ClassUniquePtrMemberInfo<BaseT, ClassT>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	llvm::Value* thisPointerAsInt = context->helper->createIntPtrConstant(reinterpret_cast<uintptr_t>(this), "ClassUniquePtrMemberInfoIntPtr");
	if (!context->helper->isPointer(parentObjectPointer))
	{
		parentObjectPointer = context->helper->convertToPointer(parentObjectPointer, memberName + "_Parent_Ptr");
	}
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		llvm::Value* thisPointer = context->helper->convertToPointer(thisPointerAsInt, "ClassUniquePtrMemberInfoPtr");
		return context->helper->createCall(LLVM::LLVMTypes::functionRetPtrArgPtr_Ptr, reinterpret_cast<uintptr_t>(&ClassUniquePtrMemberInfo<BaseT, ClassT>::getPointer), {parentObjectPointer, thisPointer}, "getUniquePtr");
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVM::LLVMTypes::pointerType, context);
#else 
	return nullptr;
#endif // ENABLE_LLVM
}


template<typename BaseT, typename BasicT>
inline std::any BasicTypeMemberInfo<BaseT, BasicT>::getMemberReference(Reflectable* base)
{
	BaseT* objectPointer = static_cast<BaseT*>(base);
	if (objectPointer != nullptr)
	{
		BasicT& value = objectPointer->*memberPointer;
		return value;
	}
	return BasicT();
}


template<typename BaseT, typename BasicT>
inline std::any BasicTypeMemberInfo<BaseT, BasicT>::getAssignableMemberReference(Reflectable* base)
{
	BaseT* objectPointer = static_cast<BaseT*>(base);
	if (objectPointer != nullptr)
	{
		BasicT& value = objectPointer->*memberPointer;
		return &value;
	}
	return (BasicT*)nullptr;
}


template<typename BaseT, typename BasicT>
inline unsigned long long BasicTypeMemberInfo<BaseT, BasicT>::getMemberPointerOffset() const
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
	return offset;
}


template<typename BaseT, typename BasicT>
inline llvm::Value* BasicTypeMemberInfo<BaseT, BasicT>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	unsigned long long offset = getMemberPointerOffset();
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{	
		llvm::Value* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
		if constexpr (std::is_same<BasicT, std::string>::value)
		{
			//std::string case (returns a pointer to the std::string)
			return context->helper->convertToPointer(addressValue, memberName, LLVM::LLVMTypes::stringPtrType);
		}
		else
		{
			//int, bool, float case	(returns by value)
			return context->helper->loadBasicType(context->helper->toLLVMType(catType), addressValue, memberName);
		}
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, context->helper->toLLVMType(catType), context);
#else 
	return nullptr;
#endif // ENABLE_LLVM
}


template<typename BaseT, typename BasicT>
inline llvm::Value* BasicTypeMemberInfo<BaseT, BasicT>::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	unsigned long long offset = getMemberPointerOffset();
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{	
		llvm::Value* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressIntValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
		if constexpr (std::is_same<BasicT, std::string>::value)
		{
			llvm::Value* lValue = context->helper->convertToPointer(addressIntValue, memberName, LLVM::LLVMTypes::stringPtrType);
			context->helper->createCall(context, &LLVM::LLVMCatIntrinsics::stringAssign, {lValue, rValue}, "assignString");
		}
		else
		{
			//int, bool, float case	(returns by value)
			llvm::Value* addressValue = context->helper->convertToPointer(addressIntValue, memberName + "_Ptr", context->helper->toLLVMPtrType(catType));
			context->helper->writeToPointer(addressValue, rValue);
		}
		return rValue;
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, context->helper->toLLVMType(catType), context);
#else 
	return nullptr;
#endif // ENABLE_LLVM
}


} //End namespace jitcat::Reflection