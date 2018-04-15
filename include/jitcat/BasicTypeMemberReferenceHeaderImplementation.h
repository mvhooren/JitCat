/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "BasicTypeMemberReference.h"
#include "MemberInfo.h"
#include "Reflectable.h"


inline void BasicTypeMemberReference<float>::setFloat(float value)
{
	if (isWritable && parentObject.get() != nullptr)
	{
		member = value;
		if (memberInfo != nullptr && parentObject.get()->getHandleSetEvents())
		{
			parentObject.get()->handleSet(memberInfo->memberName);
		}
	}
}


inline void BasicTypeMemberReference<int>::setInt(int value)
{
	if (isWritable && parentObject.get() != nullptr)
	{
		member = value;
		if (memberInfo != nullptr && parentObject.get()->getHandleSetEvents())
		{
			parentObject.get()->handleSet(memberInfo->memberName);
		}
	}
}


inline void BasicTypeMemberReference<bool>::setBool(bool value)
{
	if (isWritable && parentObject.get() != nullptr)
	{
		member = value;
		if (memberInfo != nullptr && parentObject.get()->getHandleSetEvents())
		{
			parentObject.get()->handleSet(memberInfo->memberName);
		}
	}
}


inline void BasicTypeMemberReference<std::string>::setString(const std::string& value)
{
	if (isWritable && parentObject.get() != nullptr)
	{
		member = value;
		if (memberInfo != nullptr && parentObject.get()->getHandleSetEvents())
		{
			parentObject.get()->handleSet(memberInfo->memberName);
		}
	}
}