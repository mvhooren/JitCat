#include "CustomTypeMemberInfo.h"
#include "LLVMCompileTimeContext.h"
#include "ReflectableHandle.h"

template<>
inline std::any CustomBasicTypeMemberInfo<std::string>::getMemberReference(Reflectable* base)
{
	CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(base);
	if (baseObject != nullptr)
	{
		std::string* stringPointer;
		memcpy(&stringPointer, &baseObject->data[memberOffset], sizeof(std::string*));
		std::string& value = *stringPointer;
		return value;
	}
	return std::string("");
}


template<typename T>
inline std::any CustomBasicTypeMemberInfo<T>::getMemberReference(Reflectable* base)
{
	CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(base);
	if (baseObject != nullptr)
	{
		T& value = *reinterpret_cast<T*>(&baseObject->data[memberOffset]);
		return value;
	}
	return T();
}

template<typename T>
inline llvm::Value* CustomBasicTypeMemberInfo<T>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	unsigned long long dataPointerOffset = 0;
	//Get the offset to a the "data" member of the CustomTypeInstance object that is pointed to by parentObjectPointer.
	unsigned char* CustomTypeInstance::* dataMemberPointer = &CustomTypeInstance::data;
	static_assert(sizeof(dataMemberPointer) == 4 || sizeof(dataMemberPointer) == 8);
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

	bool loadString = std::is_same<T, std::string>::value;

	auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
	{
		LLVMCodeGeneratorHelper* generatorHelper = compileContext->helper;
		//Create an llvm constant that contains the offset to "data"
		llvm::Value* dataPointerOffsetValue = generatorHelper->createIntPtrConstant(dataPointerOffset, "offsetTo_CustomTypeInstance.data" );
		//Convert pointer to int so it can be used in createAdd
		llvm::Value* dataPointerAddressInt = generatorHelper->convertToIntPtr(parentObjectPointer, memberName + "_Parent_IntPtr");
		//Add the offset to the address of the CustomTypeInstance object
		llvm::Value* dataPointerAddressValue = generatorHelper->createAdd(dataPointerAddressInt, dataPointerOffsetValue, "dataIntPtrPtr");
		//Load the data pointer stored inside the CustomTypeInstance object
		llvm::Value* dataPointer = generatorHelper->loadPointerAtAddress(dataPointerAddressValue, "dataPtr");
		//Convert to int so we can add the offset
		llvm::Value* dataPointerAsInt = generatorHelper->convertToIntPtr(dataPointer, "dataIntPtr");
		//Create a constant with the offset of this member relative to the the data pointer
		llvm::Value* memberOffsetValue = generatorHelper->createIntPtrConstant((unsigned long long)memberOffset, "offsetTo_" + memberName);
		//Add the offset to the data pointer.
		llvm::Value* addressValue = generatorHelper->createAdd(dataPointerAsInt, memberOffsetValue, memberName + "_IntPtr");
		if (loadString)
		{
			return generatorHelper->loadPointerAtAddress(addressValue, memberName, LLVMTypes::stringPtrType);
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


template<>
inline void CustomBasicTypeMemberInfo<std::string>::assign(std::any& base, const std::string& valueToSet)
{
	CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(std::any_cast<Reflectable*>(base));
	if (baseObject != nullptr)
	{
		std::string*& value = *reinterpret_cast<std::string**>(&baseObject->data[memberOffset]);
		*value = valueToSet;
	}
}


template<typename T>
inline void CustomBasicTypeMemberInfo<T>::assign(std::any& base, const T& valueToSet)
{
	CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(base);
	if (baseObject != nullptr)
	{
		T& value = *reinterpret_cast<T*>(&baseObject->data[memberOffset]);
		value = valueToSet;
	}
}



inline std::any CustomTypeObjectMemberInfo::getMemberReference(Reflectable* base)
{
	CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(base);
	if (baseObject != nullptr)
	{
		ReflectableHandle* objectPointer = reinterpret_cast<ReflectableHandle*>(&baseObject->data[memberOffset]);
		return objectPointer->get();
	}
	return (Reflectable*)nullptr;
}


inline llvm::Value* CustomTypeObjectMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCompileTimeContext* context) const
{
#ifdef ENABLE_LLVM
	unsigned long long dataPointerOffset = 0;
	//Get the offset to a the "data" member of the CustomTypeInstance object that is pointed to by parentObjectPointer.
	unsigned char* CustomTypeInstance::* dataMemberPointer = &CustomTypeInstance::data;
	
	static_assert(sizeof(dataMemberPointer) == 4 || sizeof(dataMemberPointer) == 8);

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


	auto notNullCodeGen = [=](LLVMCompileTimeContext* compileContext)
	{
		//Create an llvm constant that contains the offset to "data"
		llvm::Value* dataPointerOffsetValue = context->helper->createIntPtrConstant((unsigned long long)dataPointerOffset, "offsetTo_CustomTypeInstance.data");
		//Add the offset to the address of the CustomTypeInstance object
		llvm::Value* dataPointerAddressValue = context->helper->createAdd(parentObjectPointer, dataPointerOffsetValue, "dataPtr_IntPtr");
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
		return context->helper->createCall(LLVMTypes::functionRetPtrArgPtr, reinterpret_cast<uintptr_t>(&ReflectableHandle::staticGet), {reflectableHandle}, "getReflectable");
	};
	return context->helper->createOptionalNullCheckSelect(parentObjectPointer, notNullCodeGen, LLVMTypes::pointerType, context);
#else 
	return nullptr;
#endif //ENABLE_LLVM
}


inline void CustomTypeObjectMemberInfo::assign(std::any& base, std::any& valueToSet)
{
	CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(std::any_cast<Reflectable*>(base));
	if (baseObject != nullptr)
	{
		ReflectableHandle* handle = reinterpret_cast<ReflectableHandle*>(baseObject->data + memberOffset);
		*handle = ReflectableHandle(std::any_cast<Reflectable*>(valueToSet));
	}
}
