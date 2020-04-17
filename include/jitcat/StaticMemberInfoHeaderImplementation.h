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
			llvm::Constant* addressValue = context->helper->createIntPtrConstant(pointer, "pointerTo_" + memberName);
			return context->helper->convertToPointer(addressValue, memberName + "_Ptr");
		#else 
			return nullptr;
		#endif // ENABLE_LLVM
	}


	template<typename ContainerT>
	inline llvm::Value* StaticContainerMemberInfo<ContainerT>::generateArrayIndexCode(llvm::Value* container, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
	{
		ContainerT* nullContainer = nullptr;
		return generateIndex(nullContainer, container, index, context);
	}


	template<typename ContainerT>
	template<typename ContainerKeyType, typename ContainerItemType, typename CompareT, typename AllocatorT>
	inline typename TypeTraits<ContainerItemType>::containerItemReturnType StaticContainerMemberInfo<ContainerT>::getMapIntIndex(std::map<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>* map, int index)
	{
		int count = 0;
		for (auto& iter : (*map))
		{
			if (count == index)
			{
				if constexpr (TypeTraits<ContainerItemType>::isUniquePtr())
				{
					return iter.second.get();
				}
				else
				{
					return iter.second;
				}
			}
			count++;
		}
		return TypeTraits<ContainerItemType>::getDefaultValue();
	}


	template<typename ContainerT>
	template<typename ContainerKeyType, typename ContainerItemType, typename CompareT, typename AllocatorT>
	inline typename TypeTraits<ContainerItemType>::containerItemReturnType StaticContainerMemberInfo<ContainerT>::getMapKeyIndex(std::map<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>* map, typename TypeTraits<ContainerKeyType>::functionParameterType index)
	{
		if constexpr (std::is_same<ContainerKeyType, std::string>::value)
		{
			if (&index != nullptr)
			{
				std::string lowerCaseIdx = Tools::toLowerCase(index);
				auto iter = map->find(lowerCaseIdx);
				if (iter != map->end())
				{
					if constexpr (TypeTraits<ContainerItemType>::isUniquePtr())
					{
						return iter->second.get();
					}
					else
					{
						return iter->second;
					}
				}
			}
		}
		else
		{
			auto iter = map->find(index);
			if (iter != map->end())
			{
				if constexpr (TypeTraits<ContainerItemType>::isUniquePtr())
				{
					return iter->second.get();
				}
				else
				{
					return iter->second;
				}
			}
		}
		return TypeTraits<ContainerItemType>::getDefaultValue();
	}


	template<typename ContainerT>
	template<typename ContainerItemType, typename AllocatorT>
	inline typename TypeTraits<ContainerItemType>::containerItemReturnType StaticContainerMemberInfo<ContainerT>::getVectorIndex(std::vector<ContainerItemType, AllocatorT>* vector, int index)
	{
		if (index >= 0 && index < (int)vector->size())
		{
			if constexpr (TypeTraits<ContainerItemType>::isUniquePtr())
			{
				return vector->operator[](index).get();
			}
			else if constexpr (std::is_class_v<ContainerItemType>)
			{
				return &vector->operator[](index);
			}
			else
			{
				return vector->operator[](index);
			}
		
		}
		if constexpr (!TypeTraits<ContainerItemType>::isUniquePtr() && std::is_class_v<ContainerItemType>)
		{
			return (ContainerItemType*)nullptr;
		}
		else
		{
			return TypeTraits<ContainerItemType>::getDefaultValue();
		}
	}


	template<typename ContainerT>
	template<typename ContainerKeyType, typename ContainerItemType, typename CompareT, typename AllocatorT>
	inline llvm::Value* StaticContainerMemberInfo<ContainerT>::generateIndex(std::map<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>* map, llvm::Value* containerPtr, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
	{
		#ifdef ENABLE_LLVM
			if (!context->helper->isInt(index) || std::is_same<int, ContainerKeyType>::value)
			{
				static auto functionPointer = &StaticContainerMemberInfo<ContainerT>::getMapKeyIndex<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>;
				return context->helper->createCall(context, functionPointer, {containerPtr, index}, "getMapKeyIndex");
			}
			else
			{
				static auto functionPointer = &StaticContainerMemberInfo<ContainerT>::getMapIntIndex<ContainerKeyType, ContainerItemType, CompareT, AllocatorT>;
				return context->helper->createCall(context, functionPointer, {containerPtr, index}, "getMapIntIndex");
			}
		#else
			return nullptr;
		#endif //ENABLE_LLVM
	}


	template<typename ContainerT>
	template<typename ContainerItemType, typename AllocatorT>
	inline llvm::Value* StaticContainerMemberInfo<ContainerT>::generateIndex(std::vector<ContainerItemType, AllocatorT>* vector, llvm::Value* containerPtr, llvm::Value* index, LLVM::LLVMCompileTimeContext* context) const
	{
		#ifdef ENABLE_LLVM
			static auto functionPointer = &StaticContainerMemberInfo<ContainerT>::getVectorIndex<ContainerItemType, AllocatorT>;
			return context->helper->createCall(context, functionPointer, {containerPtr, index}, "getVectorIndex");
		#else
			return nullptr;
		#endif //ENABLE_LLVM
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
			context->helper->createCall(context, &LLVM::LLVMCatIntrinsics::stringAssign, {lValue, rValue}, "assignString");
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
