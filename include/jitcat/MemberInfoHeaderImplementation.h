/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "BasicTypeMemberReference.h"
#include "ContainerMemberReference.h"
#include "ObjectMemberReference.h"


template<typename T, typename U>
inline MemberReferencePtr ContainerMemberInfo<T, U>::getMemberReference(MemberReferencePtr& base)
{
	if (!base.isNull())
	{
		T* baseObject = static_cast<T*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			U& container = baseObject->*memberPointer;
			return new ContainerMemberReference<U>(container, this, baseObject, nestedType);
		}
	}
	return nullptr;
}


template<typename T, typename U>
inline MemberReferencePtr ClassPointerMemberInfo<T, U>::getMemberReference(MemberReferencePtr& base)
{
	if (!base.isNull())
	{
		T* baseObject = static_cast<T*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			U* member = baseObject->*memberPointer;
			return new ObjectMemberReference<U>(member, this, nestedType);
		}
	}
	return nullptr;
}


template<typename T, typename U>
inline MemberReferencePtr ClassObjectMemberInfo<T, U>::getMemberReference(MemberReferencePtr & base)
{
	if (!base.isNull())
	{
		T* baseObject = static_cast<T*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			return new ObjectMemberReference<U>(&(baseObject->*memberPointer), this, nestedType);
		}
	}
	return nullptr;
}


template<typename T, typename U>
inline MemberReferencePtr ClassUniquePtrMemberInfo<T, U>::getMemberReference(MemberReferencePtr & base)
{
	if (!base.isNull())
	{
		T* baseObject = static_cast<T*>(base->getParentObject());
		if (baseObject != nullptr)
		{
			return new ObjectMemberReference<U>((baseObject->*memberPointer).get(), this, nestedType);
		}
	}
	return nullptr;
}


template<typename T, typename U>
inline MemberReferencePtr BasicTypeMemberInfo<T, U>::getMemberReference(MemberReferencePtr& base)
{
	if (!base.isNull())
	{
		T* objectPointer = static_cast<T*>(base->getParentObject());
		if (objectPointer != nullptr)
		{
			U& value = objectPointer->*memberPointer;

			return new BasicTypeMemberReference<U>(value, this, objectPointer, isWritable);
		}
	}
	return nullptr;
}