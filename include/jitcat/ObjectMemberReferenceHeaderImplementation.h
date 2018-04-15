/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "ObjectMemberReference.h"
#include "TypeInfo.h"


template<typename T>
inline MemberReferencePtr ObjectMemberReference<T>::getMemberReference(const std::string& memberOrIndex)
{
	//Indexed container
	if (object.get() != nullptr && typeInfo != nullptr)
	{
		MemberReferencePtr thisRef(this);
		return typeInfo->getMemberReference(thisRef, memberOrIndex);
	}
	else
	{
		return nullptr;
	}
}


template<typename T>
inline const char* ObjectMemberReference<T>::getCustomTypeName() const
{
	 return typeInfo->getTypeName();
}