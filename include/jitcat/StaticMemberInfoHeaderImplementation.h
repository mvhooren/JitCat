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


namespace jitcat::Reflection
{
	template<typename ContainerT>
	inline std::any StaticContainerMemberInfo<ContainerT>::getMemberReference()
	{
		return std::any(memberPointer);
	}


	template<typename ContainerT>
	inline std::any StaticContainerMemberInfo<ContainerT>::getAssignableMemberReference()
	{
		return std::any(memberPointer);
	}


	template<typename ContainerT>
	inline llvm::Value* StaticContainerMemberInfo<ContainerT>::generateDereferenceCode(LLVM::LLVMCompileTimeContext* context) const
	{
		#ifdef ENABLE_LLVM
			static_assert(sizeof(memberPointer) == 4 || sizeof(memberPointer) == 8, "Expected a 4 or 8 byte member pointer.");
			unsigned long long pointer = 0;
			if constexpr (sizeof(memberPointer) == 4)
			{
				unsigned int smallPointer = 0;
				memcpy(&smallPointer, &memberPointer, 4);
				pointer = smallPointer;
			}
			else
			{
				memcpy(&pointer, &memberPointer, 8);
			}
			llvm::Value* addressValue = context->helper->createIntPtrConstant(pointer, "pointerTo_" + memberName);
			return context->helper->convertToPointer(addressValue, memberName + "_Ptr");
		#else 
			return nullptr;
		#endif // ENABLE_LLVM
	}


	template<typename ContainerT>
	inline llvm::Value* StaticContainerMemberInfo<ContainerT>::generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
	{
		return nullptr;
	}


	template<typename ContainerT>
	template<typename ContainerKeyType, typename ContainerItemType, typename CompareT, typename AllocatorT>
	inline llvm::Value* StaticContainerMemberInfo<ContainerT>::generateIndex(std::map<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>* map, llvm::Value* containerPtr, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
	{
		return nullptr;
	}


	template<typename ContainerT>
	template<typename ContainerItemType, typename AllocatorT>
	inline llvm::Value* StaticContainerMemberInfo<ContainerT>::generateIndex(std::vector<ContainerItemType, AllocatorT>* vector, llvm::Value* containerPtr, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
	{
		return nullptr;
	}

	
	template<typename ClassT>
	inline ClassT* StaticClassUniquePtrMemberInfo<ClassT>::getPointer(StaticClassUniquePtrMemberInfo<ClassT>* info)
	{
		return nullptr;
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
		return nullptr;
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
		return nullptr;
	}


	template<typename BasicT>
	inline llvm::Value* StaticBasicTypeMemberInfo<BasicT>::generateAssignCode(llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
	{
		return nullptr;
	}
}
