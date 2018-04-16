/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "MemberReference.h"
#include "MemberInfo.h"
#include "TypeInfo.h"


CatGenericType MemberReference::getGenericType() const
{
	if (memberInfo != nullptr)
	{
		return memberInfo->toGenericType();
	}
	else
	{
		return CatGenericType("Error");
	}
}


void MemberReference::incrementReferenceCounter()
{
	numReferences++;
}


bool MemberReference::decrementReferenceCounter()
{
	numReferences--;
	return numReferences > 0;
}


TypeMemberInfo* MemberReference::getMemberInfo() const
{
	return memberInfo;
}


MemberReferencePtr MemberReference::getMember(TypeInfo* typeInfo, MemberReferencePtr thisRef, const std::string& memberOrIndex)
{
	//Indexed container
	if (typeInfo != nullptr)
	{
		return typeInfo->getMemberReference(thisRef, memberOrIndex);
	}
	else
	{
		return nullptr;
	}
}


const char* MemberReference::getTypeName(TypeInfo* typeInfo)
{
	return typeInfo->getTypeName();
}
