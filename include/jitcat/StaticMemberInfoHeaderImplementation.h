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
#include "jitcat/STLTypeReflectors.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTools.h"
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

		std::ostringstream mangledNameStream;
		std::string classTypeName = TypeNameGetter<ClassT>::get();
		mangledNameStream << classTypeName << "* StaticClassUniquePtrMemberInfo<" << classTypeName << ">::getPointer(std::unique_ptr<" << classTypeName << ">*)";
		std::string mangledName = mangledNameStream.str();
		context->helper->defineWeakSymbol(context, reinterpret_cast<uintptr_t>(&StaticClassUniquePtrMemberInfo<ClassT>::getPointer), mangledName, false);

		return context->helper->createCall( LLVM::LLVMTypes::functionRetPtrArgPtr, {uniquePtrPtr}, false, mangledName, "getUniquePtr");
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
			return context->helper->loadBasicType(context->helper->toLLVMType(catType), addressValue, memberName);
		#else 
			return nullptr;
		#endif // ENABLE_LLVM
	}


	template<typename BasicT>
	inline llvm::Value* StaticBasicTypeMemberInfo<BasicT>::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
	{
	#ifdef ENABLE_LLVM
		llvm::Constant* addressIntValue = context->helper->createIntPtrConstant(reinterpret_cast<std::intptr_t>(memberPointer), memberName + "_IntPtr");
		llvm::Value* addressValue = context->helper->convertToPointer(addressIntValue, memberName + "_Ptr", context->helper->toLLVMPtrType(catType));
		context->helper->writeToPointer(addressValue, rValue);
		return rValue;
	#else 
		return nullptr;
	#endif // ENABLE_LLVM
	}
}
