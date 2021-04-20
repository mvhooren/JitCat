/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/CustomObject.h"
#include "jitcat/LLVMCodeGeneratorHelper.h"

#include <cassert>


using namespace jitcat;
using namespace jitcat::Reflection;


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
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = context->helper->convertToIntPtr(parentObjectPointer, "data_IntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Constant* memberOffsetValue = context->helper->createIntPtrConstant((unsigned long long)memberOffset, "offsetTo_" + memberName);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = context->helper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		//Pointer to a ReflectableHandle
		llvm::Value* reflectableHandle = context->helper->convertToPointer(addressValue, "ReflectableHandle");
		//Call function that gets the member
		return context->helper->createIntrinsicCall(context, &LLVM::CatLinkedIntrinsics::_jc_getObjectPointerFromHandle, {reflectableHandle}, "_jc_getObjectPointerFromHandle", true);

	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVM::LLVMTypes::pointerType, context);
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
		llvm::Constant* memberOffsetValue = context->helper->createIntPtrConstant((unsigned long long)memberOffset, "offsetTo_" + memberName);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = context->helper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		//Pointer to a ReflectableHandle
		llvm::Value* reflectableHandle = context->helper->convertToPointer(addressValue, "ReflectableHandle");
		//Type info of the object that will be assigned
		llvm::Constant* typeInfoConstant = context->helper->createIntPtrConstant(reinterpret_cast<uintptr_t>(catType.removeIndirection().getObjectType()), Tools::append(catType.removeIndirection().getObjectTypeName(), "_typeInfo"));
		llvm::Value* typeInfoConstantAsIntPtr = context->helper->convertToPointer(typeInfoConstant, Tools::append(catType.removeIndirection().getObjectTypeName(), "_typeInfoPtr"));
		//Call function that gets the member
		context->helper->createIntrinsicCall(context, &LLVM::CatLinkedIntrinsics::_jc_assignPointerToReflectableHandle, {reflectableHandle, rValue, typeInfoConstantAsIntPtr}, "_jc_assignPointerToReflectableHandle", true);
		return rValue;
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVM::LLVMTypes::pointerType, context);
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


std::any jitcat::Reflection::CustomTypeObjectDataMemberInfo::getMemberReference(unsigned char* base)
{
	if (base != nullptr)
	{
		uintptr_t objectPointer = reinterpret_cast<uintptr_t>(&base[memberOffset]);
		return catType.createFromRawPointer(objectPointer);
	}
	return catType.createNullPtr();
}


std::any jitcat::Reflection::CustomTypeObjectDataMemberInfo::getAssignableMemberReference(unsigned char* base)
{
	if (base != nullptr)
	{
		assert(catType.isAssignableType());
		uintptr_t objectPointer = reinterpret_cast<uintptr_t>(&base[memberOffset]);
		return catType.createFromRawPointer(objectPointer);
	}
	return catType.createNullPtr();
}


llvm::Value* jitcat::Reflection::CustomTypeObjectDataMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = context->helper->convertToIntPtr(parentObjectPointer, "data_IntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Constant* memberOffsetValue = context->helper->createIntPtrConstant((unsigned long long)memberOffset, "offsetTo_" + memberName);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = context->helper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		//Pointer to a Reflectable
		llvm::Value* reflectable = context->helper->convertToPointer(addressValue, memberName);
		//Call function that gets the member
		return reflectable;
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVM::LLVMTypes::pointerType, context);
#else 
	return nullptr;
#endif //ENABLE_LLVM
}


llvm::Value* jitcat::Reflection::CustomTypeObjectDataMemberInfo::generateAssignCode(llvm::Value* parentObjectPointer, llvm::Value* rValue, LLVM::LLVMCompileTimeContext* context) const
{
	assert(false);
	return nullptr;
}


void jitcat::Reflection::CustomTypeObjectDataMemberInfo::assign(std::any& base, std::any& valueToSet)
{
	assert(false);
}


unsigned long long jitcat::Reflection::CustomMemberInfo::getOrdinal() const
{
	return memberOffset;
}
