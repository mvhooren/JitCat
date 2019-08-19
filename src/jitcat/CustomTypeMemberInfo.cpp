#include "jitcat/CustomTypeMemberInfo.h"

#include <cassert>


using namespace jitcat;
using namespace jitcat::Reflection;


std::any CustomTypeObjectMemberInfo::getMemberReference(Reflectable* base)
{
	unsigned char* baseData = reinterpret_cast<unsigned char*>(std::any_cast<Reflectable*>(base));
	if (baseData != nullptr)
	{
		ReflectableHandle* objectPointer = reinterpret_cast<ReflectableHandle*>(&baseData[memberOffset]);
		return objectPointer->get();
	}
	return (Reflectable*)nullptr;
}


std::any CustomTypeObjectMemberInfo::getAssignableMemberReference(Reflectable* base)
{
	unsigned char* baseData = reinterpret_cast<unsigned char*>(std::any_cast<Reflectable*>(base));
	if (baseData != nullptr)
	{
		ReflectableHandle* objectPointer = reinterpret_cast<ReflectableHandle*>(&baseData[memberOffset]);
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
		llvm::Value* memberOffsetValue = context->helper->createIntPtrConstant((unsigned long long)memberOffset, "offsetTo_" + memberName);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = context->helper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		//Pointer to a ReflectableHandle
		llvm::Value* reflectableHandle = context->helper->convertToPointer(addressValue, "ReflectableHandle");
		//Call function that gets the member
		return context->helper->createCall(LLVM::LLVMTypes::functionRetPtrArgPtr, reinterpret_cast<uintptr_t>(&ReflectableHandle::staticGet), {reflectableHandle}, "getReflectable");
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
		llvm::Value* memberOffsetValue = context->helper->createIntPtrConstant((unsigned long long)memberOffset, "offsetTo_" + memberName);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = context->helper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		//Pointer to a ReflectableHandle
		llvm::Value* reflectableHandle = context->helper->convertToPointer(addressValue, "ReflectableHandle");
		//Call function that gets the member
		context->helper->createCall(context, &ReflectableHandle::staticAssign, {reflectableHandle, rValue}, "assignReflectableHandle");
		return rValue;
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVM::LLVMTypes::pointerType, context);
#else
	return nullptr;
#endif // ENABLE_LLVM
}


void CustomTypeObjectMemberInfo::assign(std::any& base, std::any& valueToSet)
{
	unsigned char* baseData = reinterpret_cast<unsigned char*>(std::any_cast<Reflectable*>(base));
	if (baseData != nullptr)
	{
		ReflectableHandle* handle = reinterpret_cast<ReflectableHandle*>(baseData + memberOffset);
		*handle = ReflectableHandle(std::any_cast<Reflectable*>(valueToSet));
	}
}


std::any jitcat::Reflection::CustomTypeObjectDataMemberInfo::getMemberReference(Reflectable* base)
{
	unsigned char* baseData = reinterpret_cast<unsigned char*>(std::any_cast<Reflectable*>(base));
	if (baseData != nullptr)
	{
		Reflectable* objectPointer = reinterpret_cast<Reflectable*>(&baseData[memberOffset]);
		return objectPointer;
	}
	return (Reflectable*)nullptr;
}


std::any jitcat::Reflection::CustomTypeObjectDataMemberInfo::getAssignableMemberReference(Reflectable* base)
{
	assert(false);
	return std::any((ReflectableHandle*)nullptr);
}


llvm::Value* jitcat::Reflection::CustomTypeObjectDataMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = context->helper->convertToIntPtr(parentObjectPointer, "data_IntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Value* memberOffsetValue = context->helper->createIntPtrConstant((unsigned long long)memberOffset, "offsetTo_" + memberName);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = context->helper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		//Pointer to a Reflectable
		llvm::Value* reflectable = context->helper->convertToPointer(addressValue, "Reflectable");
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
