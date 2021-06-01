/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Configuration.h"
#include "jitcat/JitCat.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/STLTypeReflectors.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTools.h"
#ifdef ENABLE_LLVM
	#include "jitcat/LLVMTargetConfig.h"
	#include "jitcat/LLVMTypes.h"
#endif
#include "StaticMemberInfo.h"


namespace jitcat::Reflection
{
	template<typename ClassT>
	inline StaticClassUniquePtrMemberInfo<ClassT>::StaticClassUniquePtrMemberInfo(const std::string& memberName, std::unique_ptr<ClassT>* memberPointer, 
																				  const CatGenericType& type, const char* parentTypeName): 
		StaticMemberInfo(memberName, type, parentTypeName), 
		memberPointer(memberPointer)
	{
		if (JitCat::get()->getHasPrecompiledExpression())
		{
			uintptr_t staticGetFunctionAddress = reinterpret_cast<uintptr_t>(&StaticClassUniquePtrMemberInfo<ClassT>::getPointer);
			JitCat::get()->setPrecompiledLinkedFunction(getMangledGetPointerName(), staticGetFunctionAddress);
			JitCat::get()->setPrecompiledGlobalVariable(getStaticMemberPointerVariableName(), reinterpret_cast<uintptr_t>(memberPointer));
		}
	}

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
		llvm::Value* uniquePtrPtr = context->helper->generateStaticPointerVariable(reinterpret_cast<intptr_t>(memberPointer), context, getStaticMemberPointerVariableName());

		std::string mangledName = getMangledGetPointerName();
		context->helper->defineWeakSymbol(context, reinterpret_cast<uintptr_t>(&StaticClassUniquePtrMemberInfo<ClassT>::getPointer), mangledName, false);

		return context->helper->createCall(context, context->targetConfig->getLLVMTypes().functionRetPtrArgPtr, {uniquePtrPtr}, false, mangledName, "getUniquePtr", false);
	#else 
		return nullptr;
	#endif // ENABLE_LLVM
	}


	template<typename ClassT>
	inline std::string StaticClassUniquePtrMemberInfo<ClassT>::getStaticMemberPointerVariableName() const
	{
		return Tools::append("pointerTo:", parentTypeName, "::", memberName);
	}


	template<typename ClassT>
	inline std::string StaticClassUniquePtrMemberInfo<ClassT>::getMangledGetPointerName() const
	{
		std::ostringstream mangledNameStream;
		std::string classTypeName = TypeNameGetter<ClassT>::get();
		mangledNameStream << classTypeName << "* StaticClassUniquePtrMemberInfo<" << classTypeName << ">::getPointer(std::unique_ptr<" << classTypeName << ">*)";
		return mangledNameStream.str();
	}


	template<typename BasicT>
	inline StaticBasicTypeMemberInfo<BasicT>::StaticBasicTypeMemberInfo(const std::string& memberName, BasicT* memberPointer,
																		const CatGenericType& type, const char* parentTypeName): 
		StaticMemberInfo(memberName, type, parentTypeName), 
		memberPointer(memberPointer)
	{
		if (JitCat::get()->getHasPrecompiledExpression())
		{
			JitCat::get()->setPrecompiledGlobalVariable(getStaticMemberPointerVariableName(), reinterpret_cast<uintptr_t>(memberPointer));
		}
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

			llvm::Value* addressValue = context->helper->generateStaticPointerVariable(reinterpret_cast<intptr_t>(memberPointer), context, getStaticMemberPointerVariableName());
			
			return context->helper->loadBasicType(context->helper->toLLVMType(catType), addressValue, memberName);
		#else 
			return nullptr;
		#endif // ENABLE_LLVM
	}


	template<typename BasicT>
	inline llvm::Value* StaticBasicTypeMemberInfo<BasicT>::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
	{
	#ifdef ENABLE_LLVM
		llvm::Value* addressIntValue = context->helper->generateStaticPointerVariable(reinterpret_cast<intptr_t>(memberPointer), context, getStaticMemberPointerVariableName());
		llvm::Value* addressValue = context->helper->convertToPointer(addressIntValue, memberName + "_Ptr", context->helper->toLLVMPtrType(catType));
		context->helper->writeToPointer(addressValue, rValue);
		return rValue;
	#else 
		return nullptr;
	#endif // ENABLE_LLVM
	}


	template<typename BasicT>
	inline std::string StaticBasicTypeMemberInfo<BasicT>::getStaticMemberPointerVariableName() const
	{
		return Tools::append("pointerTo:", parentTypeName, "::", memberName);
	}
}
