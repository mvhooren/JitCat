#include "jitcat/CustomTypeMemberInfo.h"

#include <cassert>


using namespace jitcat;
using namespace jitcat::Reflection;


std::any CustomTypeObjectMemberInfo::getMemberReference(Reflectable* base)
{
	CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(std::any_cast<Reflectable*>(base));
	if (baseObject != nullptr)
	{
		ReflectableHandle* objectPointer = reinterpret_cast<ReflectableHandle*>(&baseObject->data[memberOffset]);
		return objectPointer->get();
	}
	return (Reflectable*)nullptr;
}


std::any CustomTypeObjectMemberInfo::getAssignableMemberReference(Reflectable* base)
{
	CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(std::any_cast<Reflectable*>(base));
	if (baseObject != nullptr)
	{
		ReflectableHandle* objectPointer = reinterpret_cast<ReflectableHandle*>(&baseObject->data[memberOffset]);
		return objectPointer;
	}
	return (ReflectableHandle*)nullptr;
}


std::size_t CustomTypeObjectMemberInfo::getMemberOffset() const
{
	std::size_t dataPointerOffset = 0;
	//Get the offset to a the "data" member of the CustomTypeInstance object that is pointed to by parentObjectPointer.
	unsigned char* CustomTypeInstance::* dataMemberPointer = &CustomTypeInstance::data;
	
	static_assert(sizeof(dataMemberPointer) == 4 || sizeof(dataMemberPointer) == 8, "Expected pointer to CustomTypeInstance::data member to be 4 or 8 bytes in size.");

	if constexpr (sizeof(dataMemberPointer) == 4)
	{
		unsigned int memberPointer = 0;
		memcpy(&memberPointer, &dataMemberPointer, 4);
		dataPointerOffset = memberPointer;
	}
	else if constexpr (sizeof(dataMemberPointer) == 8)
	{
		memcpy(&dataPointerOffset, &dataMemberPointer, 8);
	}
	return dataPointerOffset;
}


llvm::Value* CustomTypeObjectMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	std::size_t dataPointerOffset = getMemberOffset();

	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		//Create an llvm constant that contains the offset to "data"
		llvm::Value* dataPointerOffsetValue = context->helper->createIntPtrConstant((unsigned long long)dataPointerOffset, "offsetTo_CustomTypeInstance.data");
		//Convert pointer to int so it can be used in createAdd
		llvm::Value* parentAddressInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		//Add the offset to the address of the CustomTypeInstance object
		llvm::Value* dataPointerAddressValue = context->helper->createAdd(parentAddressInt, dataPointerOffsetValue, "dataPtr_IntPtr");
		//Load the data pointer stored inside the CustomTypeInstance object
		llvm::Value* dataPointer = context->helper->loadPointerAtAddress(dataPointerAddressValue, "data_Ptr");
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = context->helper->convertToIntPtr(dataPointer, "data_IntPtr");
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
	std::size_t dataPointerOffset = getMemberOffset();

	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		//Create an llvm constant that contains the offset to "data"
		llvm::Value* dataPointerOffsetValue = context->helper->createIntPtrConstant((unsigned long long)dataPointerOffset, "offsetTo_CustomTypeInstance.data");
		//Convert pointer to int so it can be used in createAdd
		llvm::Value* parentAddressInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		//Add the offset to the address of the CustomTypeInstance object
		llvm::Value* dataPointerAddressValue = context->helper->createAdd(parentAddressInt, dataPointerOffsetValue, "dataPtr_IntPtr");
		//Load the data pointer stored inside the CustomTypeInstance object
		llvm::Value* dataPointer = context->helper->loadPointerAtAddress(dataPointerAddressValue, "data_Ptr");
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = context->helper->convertToIntPtr(dataPointer, "data_IntPtr");
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
	CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(std::any_cast<Reflectable*>(base));
	if (baseObject != nullptr)
	{
		ReflectableHandle* handle = reinterpret_cast<ReflectableHandle*>(baseObject->data + memberOffset);
		*handle = ReflectableHandle(std::any_cast<Reflectable*>(valueToSet));
	}
}


std::any jitcat::Reflection::CustomTypeObjectDataMemberInfo::getMemberReference(Reflectable* base)
{
	CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(std::any_cast<Reflectable*>(base));
	if (baseObject != nullptr)
	{
		Reflectable* objectPointer = reinterpret_cast<Reflectable*>(&baseObject->data[memberOffset]);
		return objectPointer;
	}
	return (Reflectable*)nullptr;
}


std::any jitcat::Reflection::CustomTypeObjectDataMemberInfo::getAssignableMemberReference(Reflectable* base)
{
	assert(false);
	return std::any((ReflectableHandle*)nullptr);
}


std::size_t jitcat::Reflection::CustomTypeObjectDataMemberInfo::getMemberOffset() const
{
	return 0;
}


llvm::Value* jitcat::Reflection::CustomTypeObjectDataMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVM::LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	std::size_t dataPointerOffset = getMemberOffset();

	auto notNullCodeGen = [=](LLVM::LLVMCompileTimeContext* compileContext)
	{
		//Create an llvm constant that contains the offset to "data"
		llvm::Value* dataPointerOffsetValue = context->helper->createIntPtrConstant((unsigned long long)dataPointerOffset, "offsetTo_CustomTypeInstance.data");
		//Convert pointer to int so it can be used in createAdd
		llvm::Value* parentAddressInt = context->helper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		//Add the offset to the address of the CustomTypeInstance object
		llvm::Value* dataPointerAddressValue = context->helper->createAdd(parentAddressInt, dataPointerOffsetValue, "dataPtr_IntPtr");
		//Load the data pointer stored inside the CustomTypeInstance object
		llvm::Value* dataPointer = context->helper->loadPointerAtAddress(dataPointerAddressValue, "data_Ptr");
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = context->helper->convertToIntPtr(dataPointer, "data_IntPtr");
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
