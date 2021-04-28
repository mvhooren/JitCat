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
#include "jitcat/LLVMTypes.h"
#include "jitcat/STLTypeReflectors.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeTools.h"
#include "StaticMemberInfo.h"


namespace jitcat::Reflection
{
	template<typename ClassT>
	inline StaticClassUniquePtrMemberInfo<ClassT>::StaticClassUniquePtrMemberInfo(const std::string& memberName, std::unique_ptr<ClassT>* memberPointer, 
																				  const CatGenericType& type, const char* parentTypeName): 
		StaticMemberInfo(memberName, type, parentTypeName), 
		memberPointer(memberPointer)
	{
		if constexpr (Configuration::usePreCompiledExpressions)
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
		llvm::Value* uniquePtrPtr = nullptr;
		if (!context->isPrecompilationContext)
		{
			uniquePtrPtr = context->helper->createPtrConstant(context, reinterpret_cast<uintptr_t>(memberPointer), "UniquePtrPtr");
		}
		else
		{
			llvm::GlobalVariable* globalVariable = std::static_pointer_cast<LLVM::LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalVariable(getStaticMemberPointerVariableName(), context);
			uniquePtrPtr = context->helper->loadPointerAtAddress(globalVariable, getStaticMemberPointerVariableName());
		}

		std::string mangledName = getMangledGetPointerName();
		context->helper->defineWeakSymbol(context, reinterpret_cast<uintptr_t>(&StaticClassUniquePtrMemberInfo<ClassT>::getPointer), mangledName, false);

		return context->helper->createCall(context, LLVM::LLVMTypes::functionRetPtrArgPtr, {uniquePtrPtr}, false, mangledName, "getUniquePtr", false);
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
		if constexpr (Configuration::usePreCompiledExpressions)
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

			llvm::Value* addressValue;
			if (!context->isPrecompilationContext)
			{
				addressValue = context->helper->constantToValue(context->helper->createIntPtrConstant(context, reinterpret_cast<std::intptr_t>(memberPointer), memberName + "_IntPtr"));
			}
			else
			{
				llvm::GlobalVariable* globalVariable = std::static_pointer_cast<LLVM::LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalVariable(getStaticMemberPointerVariableName(), context);
				addressValue = context->helper->loadPointerAtAddress(globalVariable, getStaticMemberPointerVariableName());
			}
			
			return context->helper->loadBasicType(context->helper->toLLVMType(catType), addressValue, memberName);
		#else 
			return nullptr;
		#endif // ENABLE_LLVM
	}


	template<typename BasicT>
	inline llvm::Value* StaticBasicTypeMemberInfo<BasicT>::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
	{
	#ifdef ENABLE_LLVM
		llvm::Value* addressIntValue;
		if (!context->isPrecompilationContext)
		{
			addressIntValue = context->helper->constantToValue(context->helper->createIntPtrConstant(context, reinterpret_cast<std::intptr_t>(memberPointer), memberName + "_IntPtr"));
		}
		else
		{
			llvm::GlobalVariable* globalVariable = std::static_pointer_cast<LLVM::LLVMPrecompilationContext>(context->catContext->getPrecompilationContext())->defineGlobalVariable(getStaticMemberPointerVariableName(), context);
			addressIntValue = context->helper->loadPointerAtAddress(globalVariable, getStaticMemberPointerVariableName());
		}
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
