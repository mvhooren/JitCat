/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/LLVMCompileTimeContext.h"
#include "jitcat/ReflectableHandle.h"

namespace jitcat::Reflection
{


template<typename T>
inline std::any CustomBasicTypeMemberInfo<T>::getMemberReference(Reflectable* base)
{
	unsigned char* baseData = reinterpret_cast<unsigned char*>(base);
	if (baseData != nullptr)
	{
		T& value = *reinterpret_cast<T*>(&baseData[memberOffset]);
		return value;
	}
	return T();
}


template<typename T>
inline std::any CustomBasicTypeMemberInfo<T>::getAssignableMemberReference(Reflectable* base)
{
	unsigned char* baseData = reinterpret_cast<unsigned char*>(base);
	if (baseData != nullptr)
	{
		T* value = reinterpret_cast<T*>(&baseData[memberOffset]);
		return value;
	}
	return (T*)nullptr;
}


template<typename T>
inline llvm::Value* CustomBasicTypeMemberInfo<T>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM

	static const bool loadString = std::is_same<T, std::string>::value;

	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		LLVM::LLVMCodeGeneratorHelper* generatorHelper = compileContext->helper;
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = generatorHelper->convertToIntPtr(parentObjectPointer, "dataIntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Value* memberOffsetValue = generatorHelper->createIntPtrConstant((unsigned long long)memberOffset, "offsetTo_" + memberName);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = generatorHelper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		if constexpr (loadString)
		{
			return generatorHelper->convertToPointer(addressValue, memberName, LLVM::LLVMTypes::stringPtrType);
		}
		else
		{
			return generatorHelper->loadBasicType(generatorHelper->toLLVMType(catType), addressValue, memberName);
		}
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, context->helper->toLLVMType(catType), context);
#else 
	return nullptr;
#endif //ENABLE_LLVM
}


template<typename T>
inline llvm::Value* CustomBasicTypeMemberInfo<T>::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	static const bool isString = std::is_same<T, std::string>::value;

	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		LLVM::LLVMCodeGeneratorHelper* generatorHelper = compileContext->helper;
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = generatorHelper->convertToIntPtr(parentObjectPointer, "dataIntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Value* memberOffsetValue = generatorHelper->createIntPtrConstant((unsigned long long)memberOffset, "offsetTo_" + memberName);
		//Add the offset to the data pointer.
		llvm::Value* addressIntValue = generatorHelper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		if constexpr (isString)
		{
			//llvm::Value* lValue = generatorHelper->loadPointerAtAddress(addressIntValue, memberName, LLVM::LLVMTypes::stringPtrType);
			llvm::Value* lValue = generatorHelper->convertToPointer(addressIntValue, memberName, LLVM::LLVMTypes::stringPtrType);
			context->helper->createCall(context, &LLVM::LLVMCatIntrinsics::stringAssign, {lValue, rValue}, "stringAssign");
		}
		else
		{
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


template<typename T>
inline void CustomBasicTypeMemberInfo<T>::assign(std::any& base, const T& valueToSet)
{
	unsigned char* baseData = reinterpret_cast<unsigned char*>(std::any_cast<Reflectable*>(base));
	if (baseData != nullptr)
	{
		T& value = *reinterpret_cast<T*>(&baseData[memberOffset]);
		value = valueToSet;
	}
}


} //End namespace jitcat::Reflection