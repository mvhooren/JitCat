/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMTypes.h"
#include "jitcat/Tools.h"
#include "StaticMemberInfo.h"


namespace jitcat::Reflection
{
	template<typename ClassT>
	inline ClassT* StaticClassUniquePtrMemberInfo<ClassT>::getPointer(std::unique_ptr<ClassT>* info)
	{
		return info->get();
	}


	template<typename ClassT>
	inline std::any StaticClassUniquePtrMemberInfo<ClassT>::getMemberReference()
	{
		return std::any(memberPointer->get());
	}


	template<typename ClassT>
	inline std::any StaticClassUniquePtrMemberInfo<ClassT>::getAssignableMemberReference()
	{
		return std::any((ClassT**)nullptr);
	}


	template<typename ClassT>
	inline llvm::Value* StaticClassUniquePtrMemberInfo<ClassT>::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
	{
	#ifdef ENABLE_LLVM
		llvm::Value* uniquePtrPtr = context->helper->createPtrConstant(reinterpret_cast<uintptr_t>(memberPointer), "UniquePtrPtr");
		return context->helper->createCall( LLVM::LLVMTypes::functionRetPtrArgPtr, reinterpret_cast<uintptr_t>(&StaticClassUniquePtrMemberInfo<ClassT>::getPointer), {uniquePtrPtr}, "getUniquePtr");
	#else 
		return nullptr;
	#endif // ENABLE_LLVM
	}


	template<typename BasicT>
	inline std::any StaticBasicTypeMemberInfo<BasicT>::getMemberReference()
	{
		return std::any(*memberPointer);
	}


	template<typename BasicT>
	inline std::any StaticBasicTypeMemberInfo<BasicT>::getAssignableMemberReference()
	{
		return std::any(memberPointer);
	}


	template<typename BasicT>
	inline llvm::Value* StaticBasicTypeMemberInfo<BasicT>::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
	{
		#ifdef ENABLE_LLVM
			llvm::Constant* addressValue = context->helper->createIntPtrConstant(reinterpret_cast<std::intptr_t>(memberPointer), memberName + "_IntPtr");
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
		#else 
			return nullptr;
		#endif // ENABLE_LLVM
	}


	template<typename BasicT>
	inline llvm::Value* StaticBasicTypeMemberInfo<BasicT>::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
	{
	#ifdef ENABLE_LLVM
		llvm::Constant* addressIntValue = context->helper->createIntPtrConstant(reinterpret_cast<std::intptr_t>(memberPointer), memberName + "_IntPtr");
		if constexpr (std::is_same<BasicT, std::string>::value)
		{
			llvm::Value* lValue = context->helper->convertToPointer(addressIntValue, memberName, LLVM::LLVMTypes::stringPtrType);
			context->helper->createIntrinsicCall(context, &LLVM::LLVMCatIntrinsics::stringAssign, {lValue, rValue}, "assignString");
		}
		else
		{
			//int, bool, float case	(returns by value)
			llvm::Value* addressValue = context->helper->convertToPointer(addressIntValue, memberName + "_Ptr", context->helper->toLLVMPtrType(catType));
			context->helper->writeToPointer(addressValue, rValue);
		}
		return rValue;
	#else 
		return nullptr;
	#endif // ENABLE_LLVM
	}
}
