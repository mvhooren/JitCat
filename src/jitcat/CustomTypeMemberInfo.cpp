/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Configuration.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/CustomObject.h"
#include "jitcat/JitCat.h"
#include "jitcat/LLVMCodeGeneratorHelper.h"
#ifdef ENABLE_LLVM
	#include "jitcat/LLVMTargetConfig.h"
#endif
#include <cassert>


using namespace jitcat;
using namespace jitcat::Reflection;


CustomMemberInfo::CustomMemberInfo(const std::string& memberName, std::size_t memberOffset, const CatGenericType& type, const char* parentTypeName): 
	TypeMemberInfo(memberName, type), 
	parentTypeName(parentTypeName),
	memberOffset(memberOffset)
{
	if (JitCat::get()->getHasPrecompiledExpression())
	{
		JitCat::get()->setPrecompiledGlobalVariable(getMemberOffsetVariableName(), memberOffset);
	}
}


unsigned long long CustomMemberInfo::getOrdinal() const
{
	return memberOffset;
}


std::string CustomMemberInfo::getMemberOffsetVariableName() const
{
	return Tools::append("offsetTo ", parentTypeName, "::", memberName);
}


std::any CustomTypeObjectMemberInfo::getMemberReference(unsigned char* base)
{
	if (base != nullptr)
	{
		ReflectableHandle* objectPointer = reinterpret_cast<ReflectableHandle*>(&base[memberOffset]);
		return catType.createFromRawPointer(reinterpret_cast<uintptr_t>(objectPointer->get()));
	}
	return catType.createNullPtr();
}


std::any CustomTypeObjectMemberInfo::getAssignableMemberReference(unsigned char* base)
{
	if (base != nullptr)
	{
		ReflectableHandle* objectPointer = reinterpret_cast<ReflectableHandle*>(&base[memberOffset]);
		return objectPointer;
	}
	return (ReflectableHandle*)nullptr;
}


llvm::Value* CustomTypeObjectMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	llvm::PointerType* memberPointerType = context->helper->toLLVMPtrType(getType());
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = context->helper->convertToIntPtr(parentObjectPointer, "data_IntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Value* memberOffsetValue = context->helper->createOffsetGlobalValue(context, getMemberOffsetVariableName(), memberOffset);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = context->helper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		//Pointer to a ReflectableHandle
		llvm::Value* reflectableHandle = context->helper->convertToPointer(addressValue, "ReflectableHandle", context->targetConfig->getLLVMTypes().pointerType);
		//Call function that gets the member
		llvm::Value* returnValue = context->helper->createIntrinsicCall(context, &LLVM::CatLinkedIntrinsics::_jc_getObjectPointerFromHandle, {reflectableHandle}, "_jc_getObjectPointerFromHandle", true);
		return context->helper->convertToPointer(returnValue, "CastToPointerType", memberPointerType);

	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, memberPointerType, context);
#else 
	return nullptr;
#endif //ENABLE_LLVM
}


llvm::Value* CustomTypeObjectMemberInfo::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = context->helper->convertToIntPtr(parentObjectPointer, "data_IntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Value* memberOffsetValue = context->helper->createOffsetGlobalValue(context, getMemberOffsetVariableName(), memberOffset);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = context->helper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		//Pointer to a ReflectableHandle
		llvm::Value* reflectableHandle = context->helper->convertToPointer(addressValue, "ReflectableHandle", context->targetConfig->getLLVMTypes().pointerType);
		//Type info of the object that will be assigned
		llvm::Value* typeInfoConstantAsIntPtr = context->helper->createTypeInfoGlobalValue(context, catType.removeIndirection().getObjectType());
		//Call function that gets the member
		context->helper->createIntrinsicCall(context, &LLVM::CatLinkedIntrinsics::_jc_assignPointerToReflectableHandle, {reflectableHandle, rValue, typeInfoConstantAsIntPtr}, "_jc_assignPointerToReflectableHandle", true);
		return rValue;
	};
	llvm::PointerType* memberPointerType = context->helper->toLLVMPtrType(getType());
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, memberPointerType, context);
#else
	return nullptr;
#endif // ENABLE_LLVM
}


void CustomTypeObjectMemberInfo::assign(std::any& base, std::any& valueToSet)
{
	unsigned char* baseData = reinterpret_cast<unsigned char*>(std::any_cast<CustomObject*>(base));
	if (baseData != nullptr)
	{
		ReflectableHandle* handle = reinterpret_cast<ReflectableHandle*>(baseData + memberOffset);
		*handle = ReflectableHandle(reinterpret_cast<unsigned char*>(catType.getRawPointer(valueToSet)), catType.removeIndirection().getObjectType());
	}
}


std::any CustomTypeObjectDataMemberInfo::getMemberReference(unsigned char* base)
{
	if (base != nullptr)
	{
		uintptr_t objectPointer = reinterpret_cast<uintptr_t>(&base[memberOffset]);
		return catType.createFromRawPointer(objectPointer);
	}
	return catType.createNullPtr();
}


std::any CustomTypeObjectDataMemberInfo::getAssignableMemberReference(unsigned char* base)
{
	if (base != nullptr)
	{
		assert(catType.isAssignableType());
		uintptr_t objectPointer = reinterpret_cast<uintptr_t>(&base[memberOffset]);
		return catType.createFromRawPointer(objectPointer);
	}
	return catType.createNullPtr();
}


llvm::Value* CustomTypeObjectDataMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	llvm::PointerType* memberPointerType = context->helper->toLLVMPtrType(getType());
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = context->helper->convertToIntPtr(parentObjectPointer, "data_IntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Value* memberOffsetValue = context->helper->createOffsetGlobalValue(context, getMemberOffsetVariableName(), memberOffset);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = context->helper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		//Pointer to a Reflectable
		llvm::Value* reflectable = context->helper->convertToPointer(addressValue, memberName, memberPointerType);
		//Call function that gets the member
		return reflectable;
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, memberPointerType, context);
#else 
	return nullptr;
#endif //ENABLE_LLVM
}


llvm::Value* CustomTypeObjectDataMemberInfo::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
	assert(false);
	return nullptr;
}


void jitcat::Reflection::CustomTypeObjectDataMemberInfo::assign(std::any& base, std::any& valueToSet)
{
	assert(false);
}

