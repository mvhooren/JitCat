/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/CustomObject.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/LLVMCodeGeneratorHelper.h"
#include "jitcat/ReflectableHandle.h"

namespace jitcat::Reflection
{


template<typename BasicT>
inline std::any CustomBasicTypeMemberInfo<BasicT>::getMemberReference(unsigned char* base)
{
	if (base != nullptr)
	{
		BasicT& value = *reinterpret_cast<BasicT*>(&base[memberOffset]);
		return value;
	}
	return BasicT();
}


template<typename BasicT>
inline std::any CustomBasicTypeMemberInfo<BasicT>::getAssignableMemberReference(unsigned char* base)
{
	if (base != nullptr)
	{
		BasicT* value = reinterpret_cast<BasicT*>(&base[memberOffset]);
		return value;
	}
	return (BasicT*)nullptr;
}


template<typename BasicT>
inline llvm::Value* CustomBasicTypeMemberInfo<BasicT>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM

	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		LLVM::LLVMCodeGeneratorHelper* generatorHelper = compileContext->helper;
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = generatorHelper->convertToIntPtr(parentObjectPointer, "dataIntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Value* memberOffsetValue = context->helper->createOffsetGlobalValue(context, getMemberOffsetVariableName(), memberOffset);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = generatorHelper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		return generatorHelper->loadBasicType(generatorHelper->toLLVMType(catType), addressValue, memberName);
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, context->helper->toLLVMType(catType), context);
#else 
	return nullptr;
#endif //ENABLE_LLVM
}


template<typename BasicT>
inline llvm::Value* CustomBasicTypeMemberInfo<BasicT>::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		LLVM::LLVMCodeGeneratorHelper* generatorHelper = compileContext->helper;
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = generatorHelper->convertToIntPtr(parentObjectPointer, "dataIntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Value* memberOffsetValue = context->helper->createOffsetGlobalValue(context, getMemberOffsetVariableName(), memberOffset);
		//Add the offset to the data pointer.
		llvm::Value* addressIntValue = generatorHelper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		llvm::Value* addressValue = context->helper->convertToPointer(addressIntValue, memberName + "_Ptr", context->helper->toLLVMPtrType(catType));
		context->helper->writeToPointer(addressValue, rValue);
		return rValue;
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, context->helper->toLLVMType(catType), context);
#else
	return nullptr;
#endif // ENABLE_LLVM
}


template<typename BasicT>
inline void CustomBasicTypeMemberInfo<BasicT>::assign(std::any& base, const BasicT& valueToSet)
{
	unsigned char* baseData = reinterpret_cast<unsigned char*>(std::any_cast<CustomObject*>(base));
	if (baseData != nullptr)
	{
		BasicT& value = *reinterpret_cast<BasicT*>(&baseData[memberOffset]);
		value = valueToSet;
	}
}


} //End namespace jitcat::Reflection