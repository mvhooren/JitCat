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
#include "jitcat/STLTypeReflectors.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTools.h"

#include <sstream>

namespace jitcat::Reflection
{

template<typename BaseT, typename MemberT>
inline unsigned long long getOffset(MemberT BaseT::* memberPointer)
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
inline std::any ClassPointerMemberInfo<BaseT, ClassT>::getMemberReference(unsigned char* base)
{
	BaseT* baseObject = reinterpret_cast<BaseT*>(base);
	if (baseObject != nullptr)
	{
		return baseObject->*memberPointer;
	}
	return static_cast<ClassT*>(nullptr);
}


template<typename BaseT, typename ClassT>
inline std::any ClassPointerMemberInfo<BaseT, ClassT>::getAssignableMemberReference(unsigned char* base)
{
	BaseT* baseObject = reinterpret_cast<BaseT*>(base);
	if (baseObject != nullptr)
	{
		return &(baseObject->*memberPointer);
	}
	return static_cast<ClassT**>(nullptr);
}


template<typename BaseT, typename ClassT>
inline unsigned long long ClassPointerMemberInfo<BaseT, ClassT>::getMemberPointerOffset() const
{
	return getOffset(memberPointer);
}


template<typename BaseT, typename ClassT>
inline llvm::Value* ClassPointerMemberInfo<BaseT, ClassT>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	unsigned long long offset = getMemberPointerOffset();
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		llvm::Constant* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
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
		llvm::Constant* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
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
inline unsigned long long ClassPointerMemberInfo<BaseT, ClassT>::getOrdinal() const
{
	return getMemberPointerOffset();
}


template<typename BaseT, typename ClassT>
inline std::any ClassObjectMemberInfo<BaseT, ClassT>::getMemberReference(unsigned char* base)
{
	BaseT* baseObject = reinterpret_cast<BaseT*>(base);
	if (baseObject != nullptr)
	{
		ClassT* returnVal = &(baseObject->*memberPointer);
		return returnVal;
	}
	return static_cast<ClassT*>(nullptr);
}


template<typename BaseT, typename ClassT>
inline std::any ClassObjectMemberInfo<BaseT, ClassT>::getAssignableMemberReference(unsigned char* base)
{
	//Not supported for now (would require implementing calling of operator= on target object)
	return getMemberReference(base);
}


template<typename BaseT, typename ClassT>
inline llvm::Value* ClassObjectMemberInfo<BaseT, ClassT>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	unsigned long long offset = getOffset(memberPointer);;
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		llvm::Constant* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
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
inline unsigned long long ClassObjectMemberInfo<BaseT, ClassT>::getOrdinal() const
{
	return getOffset(memberPointer);
}


template<typename BaseT, typename ClassT>
inline ClassT* ClassUniquePtrMemberInfo<BaseT, ClassT>::getPointer(BaseT* parentObject, ClassUniquePtrMemberInfo<BaseT, ClassT>* info)
 {
	std::unique_ptr<ClassT> BaseT::* memberPointer = info->memberPointer;
	return (parentObject->*memberPointer).get();
}


template<typename BaseT, typename ClassT>
inline std::any ClassUniquePtrMemberInfo<BaseT, ClassT>::getMemberReference(unsigned char* base)
{
	BaseT* baseObject = reinterpret_cast<BaseT*>(base);
	if (baseObject != nullptr)
	{
		return (baseObject->*memberPointer).get();
	}
	return static_cast<ClassT*>(nullptr);
}


template<typename BaseT, typename ClassT>
inline std::any ClassUniquePtrMemberInfo<BaseT, ClassT>::getAssignableMemberReference(unsigned char* base)
{
	//Cannot assing unique_ptr, this would transfer ownership and potentially delete the pointer at some point. Bad idea.
	//The pointer may for example have come from another unique_ptr.
	return std::any((ClassT*)nullptr);
}


template<typename BaseT, typename ClassT>
inline llvm::Value* ClassUniquePtrMemberInfo<BaseT, ClassT>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	llvm::Constant* thisPointerAsInt = context->helper->createIntPtrConstant(reinterpret_cast<uintptr_t>(this), "ClassUniquePtrMemberInfoIntPtr");
	if (!context->helper->isPointer(parentObjectPointer))
	{
		parentObjectPointer = context->helper->convertToPointer(parentObjectPointer, memberName + "_Parent_Ptr");
	}
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		std::ostringstream mangledNameStream;
		std::string baseTypeName = TypeNameGetter<BaseT>::get();
		std::string classTypeName = TypeNameGetter<ClassT>::get();
		mangledNameStream << classTypeName << "* ClassUniquePtrMemberInfo<" << baseTypeName << ", " << classTypeName << ">::getPointer(" << baseTypeName << "*, ClassUniquePtrMemberInfo<" << baseTypeName << ", " << classTypeName << ">*)";
		std::string mangledName = mangledNameStream.str();
		context->helper->defineWeakSymbol(reinterpret_cast<uintptr_t>(&ClassUniquePtrMemberInfo<BaseT, ClassT>::getPointer), mangledName);
		
		llvm::Value* thisPointer = context->helper->convertToPointer(thisPointerAsInt, "ClassUniquePtrMemberInfoPtr");
		return context->helper->createCall(LLVM::LLVMTypes::functionRetPtrArgPtr_Ptr, {parentObjectPointer, thisPointer}, mangledName, "getUniquePtr");
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVM::LLVMTypes::pointerType, context);
#else 
	return nullptr;
#endif // ENABLE_LLVM
}


template<typename BaseT, typename ClassT>
inline unsigned long long ClassUniquePtrMemberInfo<BaseT, ClassT>::getOrdinal() const
{
	return getOffset(memberPointer);
}


template<typename BaseT, typename BasicT>
inline std::any BasicTypeMemberInfo<BaseT, BasicT>::getMemberReference(unsigned char* base)
{
	BaseT* objectPointer = reinterpret_cast<BaseT*>(base);
	if (objectPointer != nullptr)
	{
		BasicT& value = objectPointer->*memberPointer;
		return value;
	}
	return BasicT();
}


template<typename BaseT, typename BasicT>
inline std::any BasicTypeMemberInfo<BaseT, BasicT>::getAssignableMemberReference(unsigned char* base)
{
	BaseT* objectPointer = reinterpret_cast<BaseT*>(base);
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
	return getOffset(memberPointer);
}


template<typename BaseT, typename BasicT>
inline llvm::Value* BasicTypeMemberInfo<BaseT, BasicT>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	unsigned long long offset = getMemberPointerOffset();
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{	
		llvm::Constant* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
		return context->helper->loadBasicType(context->helper->toLLVMType(catType), addressValue, memberName);
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
		llvm::Constant* memberOffset = context->helper->createIntPtrConstant(offset, "offsetTo_" + memberName);
		llvm::Value* parentObjectPointerInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		llvm::Value* addressIntValue = context->helper->createAdd(parentObjectPointerInt, memberOffset, memberName + "_IntPtr");
		llvm::Value* addressValue = context->helper->convertToPointer(addressIntValue, memberName + "_Ptr", context->helper->toLLVMPtrType(catType));
		context->helper->writeToPointer(addressValue, rValue);
		return rValue;
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, context->helper->toLLVMType(catType), context);
#else 
	return nullptr;
#endif // ENABLE_LLVM
}

template<typename BaseT, typename BasicT>
inline unsigned long long BasicTypeMemberInfo<BaseT, BasicT>::getOrdinal() const
{
	return getMemberPointerOffset();
}


} //End namespace jitcat::Reflection