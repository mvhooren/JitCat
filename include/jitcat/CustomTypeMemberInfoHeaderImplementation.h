#include "CustomTypeMemberInfo.h"


template<>
inline MemberReferencePtr CustomBasicTypeMemberInfo<std::string>::getMemberReference(MemberReferencePtr& base)
{
	if (!base.isNull())
	{
		CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			std::string* stringPointer;
			memcpy(&stringPointer, &baseObject->data[memberOffset], sizeof(std::string*));
			std::string& value = *stringPointer;
			return new BasicTypeMemberReference<std::string>(value, this, baseObject, isWritable);
		}
	}
	return nullptr;
}


template<typename T>
inline MemberReferencePtr CustomBasicTypeMemberInfo<T>::getMemberReference(MemberReferencePtr& base)
{
	if (!base.isNull())
	{
		CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			T& value = *reinterpret_cast<T*>(&baseObject->data[memberOffset]);
			return new BasicTypeMemberReference<T>(value, this, baseObject, isWritable);
		}
	}
	return nullptr;
}

template<typename T>
inline llvm::Value* CustomBasicTypeMemberInfo<T>::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCodeGeneratorHelper* generatorHelper) const
{
	unsigned int dataPointerOffset = 0;
	//Get the offset to a the "data" member of the CustomTypeInstance object that is pointed to by parentObjectPointer.
	unsigned char* CustomTypeInstance::* dataMemberPointer = &CustomTypeInstance::data;
	memcpy(&dataPointerOffset, &dataMemberPointer, 4);
	static_assert(sizeof(dataMemberPointer) == 4);
	//Create an llvm constant that contains the offset to "data"
	llvm::Value* dataPointerOffsetValue = generatorHelper->createIntPtrConstant((unsigned long long)dataPointerOffset);
	//Add the offset to the address of the CustomTypeInstance object
	llvm::Value* dataPointerAddressValue = generatorHelper->createAdd(parentObjectPointer, dataPointerOffsetValue);
	//Load the data pointer stored inside the CustomTypeInstance object
	llvm::Value* dataPointer = generatorHelper->loadPointerAtAddress(dataPointerAddressValue);
	//Convert to int so we can add the offset
	llvm::Value* dataPointerAsInt = generatorHelper->convertToIntPtr(dataPointer);
	//Create a constant with the offset of this member relative to the the data pointer
	llvm::Value* memberOffsetValue = generatorHelper->createIntPtrConstant((unsigned long long)memberOffset);
	//Add the offset to the data pointer.
	llvm::Value* addressValue = generatorHelper->createAdd(dataPointerAsInt, memberOffsetValue);
	if constexpr (std::is_same<T, std::string>::value)
	{
		//std::string case (returns a pointer to the std::string)
		return generatorHelper->loadPointerAtAddress(addressValue);
	}
	else
	{
		//int, bool, float case	(returns by value)
		return generatorHelper->loadBasicType(generatorHelper->toLLVMType(catType), addressValue);
	}
}


template<>
inline void CustomBasicTypeMemberInfo<std::string>::assign(MemberReferencePtr& base, const std::string& valueToSet)
{
	if (!base.isNull())
	{
		CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			std::string*& value = *reinterpret_cast<std::string**>(&baseObject->data[memberOffset]);
			*value = valueToSet;
		}
	}
}


template<typename T>
inline void CustomBasicTypeMemberInfo<T>::assign(MemberReferencePtr& base, const T& valueToSet)
{
	if (!base.isNull())
	{
		CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			T& value = *reinterpret_cast<T*>(&baseObject->data[memberOffset]);
			value = valueToSet;
		}
	}
}


inline MemberReferencePtr CustomTypeObjectMemberInfo::getMemberReference(MemberReferencePtr& base)
{
	if (!base.isNull())
	{
		CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			MemberReferencePtr* objectPointer;
			memcpy(&objectPointer, &baseObject->data[memberOffset], sizeof(MemberReferencePtr*));
			return MemberReferencePtr(*objectPointer, objectPointer);
		}
	}
	return nullptr;
}


inline llvm::Value* CustomTypeObjectMemberInfo::generateDereferenceCode(llvm::Value* parentObjectPointer, LLVMCodeGeneratorHelper* generatorHelper) const
{
	unsigned int dataPointerOffset = 0;
	//Get the offset to a the "data" member of the CustomTypeInstance object that is pointed to by parentObjectPointer.
	unsigned char* CustomTypeInstance::* dataMemberPointer = &CustomTypeInstance::data;
	memcpy(&dataPointerOffset, &dataMemberPointer, 4);
	static_assert(sizeof(dataMemberPointer) == 4);
	//Create an llvm constant that contains the offset to "data"
	llvm::Value* dataPointerOffsetValue = generatorHelper->createIntPtrConstant((unsigned long long)dataPointerOffset);
	//Add the offset to the address of the CustomTypeInstance object
	llvm::Value* dataPointerAddressValue = generatorHelper->createAdd(parentObjectPointer, dataPointerOffsetValue);
	//Load the data pointer stored inside the CustomTypeInstance object
	llvm::Value* dataPointer = generatorHelper->loadPointerAtAddress(dataPointerAddressValue);
	//Convert to int so we can add the offset
	llvm::Value* dataPointerAsInt = generatorHelper->convertToIntPtr(dataPointer);
	//Create a constant with the offset of this member relative to the the data pointer
	llvm::Value* memberOffsetValue = generatorHelper->createIntPtrConstant((unsigned long long)memberOffset);
	//Add the offset to the data pointer.
	llvm::Value* addressValue = generatorHelper->createAdd(dataPointerAsInt, memberOffsetValue);
	return generatorHelper->loadPointerAtAddress(addressValue);
}


inline void CustomTypeObjectMemberInfo::assign(MemberReferencePtr& base, MemberReferencePtr valueToSet)
{
	if (!base.isNull())
	{
		CustomTypeInstance* baseObject = static_cast<CustomTypeInstance*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			MemberReferencePtr*& value = *reinterpret_cast<MemberReferencePtr**>(&baseObject->data[memberOffset]);
			*value = valueToSet;
			value->setOriginalReference(value);
		}
	}
}
