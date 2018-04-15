/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "BasicTypeMemberReference.h"
#include "CustomTypeInstance.h"
#include "MemberInfo.h"
#include "ObjectMemberReference.h"


template<typename T>
struct CustomBasicTypeMemberInfo: public TypeMemberInfo
{
	CustomBasicTypeMemberInfo(const std::string& memberName, unsigned int memberOffset, CatType type, bool isConst, bool isWritable): TypeMemberInfo(memberName, type, isConst, isWritable), memberOffset(memberOffset) {}
	inline virtual MemberReferencePtr getMemberReference(MemberReferencePtr& base);
	void assign(MemberReferencePtr& base, const T& valueToSet);
	unsigned int memberOffset;
};


//Implements a TypeMemberInfo for class/struct types that are reflectable.
struct CustomTypeObjectMemberInfo: public TypeMemberInfo
{
	CustomTypeObjectMemberInfo(const std::string& memberName, unsigned int memberOffset, TypeInfo* type, bool isConst): TypeMemberInfo(memberName, type, isConst, false), memberOffset(memberOffset) {}

	inline virtual MemberReferencePtr getMemberReference(MemberReferencePtr& base);
	void assign(MemberReferencePtr& base, MemberReferencePtr valueToSet);
	
	unsigned int memberOffset;
};


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
