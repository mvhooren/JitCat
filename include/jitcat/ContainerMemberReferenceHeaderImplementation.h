/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "ObjectMemberReference.h"
#include "TypeTraits.h"


template <typename U>
inline MemberReferencePtr ContainerMemberReference<std::vector<U> >::getMemberReference(const std::string& index)
{
	//Indexed container
	if (parentObject.getIsValid())
	{
		int intIndex = Tools::convert<int>(index);
		if (intIndex < (int)container.size() && intIndex >= 0
			&& itemTypeInfo != nullptr)
		{
			return new ObjectMemberReference<typename TypeTraits<U>::type>(TypeTraits<U>::getPointer(container[intIndex]), nullptr, itemTypeInfo);
		}
	}
	return nullptr;
}


template <typename U>
inline MemberReferencePtr ContainerMemberReference<std::vector<U> >::getArrayItemReference(int index)
{
	//Indexed container
	if (parentObject.getIsValid())
	{
		if (index < (int)container.size() && index >= 0
			&& itemTypeInfo != nullptr)
		{
			return new ObjectMemberReference<typename TypeTraits<U>::type>(TypeTraits<U>::getPointer(container[index]), nullptr, itemTypeInfo);
		}
	}
	return nullptr;
}


template <typename U>
const std::string ContainerMemberReference<std::vector<U> >::getValueAsString() const
{
	if (parentObject.getIsValid()) { return Tools::append(container.size(), "x", getTypeName(itemTypeInfo)); }
	else { return "invalid"; }
}


template <typename U>
inline const char* ContainerMemberReference<std::vector<U> >::getCustomTypeName() const 
{
	return getTypeName(itemTypeInfo); 

}


template <typename U>
inline MemberReferencePtr ContainerMemberReference<std::map<std::string, U> >::getMemberReference(const std::string& index)
{
	if (parentObject.getIsValid())
	{
		//Indexed container
		if (itemTypeInfo != nullptr)
		{
			typename std::map<std::string, U>::iterator iter = container.find(Tools::toLowerCase(index));
			if (iter != container.end())
			{
				return new ObjectMemberReference<typename TypeTraits<U>::type>(TypeTraits<U>::getPointer(iter->second), nullptr, itemTypeInfo);
			}
		}
	}
	return nullptr;
}


template <typename U>
inline MemberReferencePtr ContainerMemberReference<std::map<std::string, U> >::getArrayItemReference(int index)
{
	if (parentObject.getIsValid() && itemTypeInfo != nullptr)
	{
		typename std::map<std::string, U>::iterator endIter = container.end();
		unsigned int i = 0;
		for (typename std::map<std::string, U>::iterator iter = container.begin(); iter != endIter; ++iter)
		{
			if (i == index)
			{
				return new ObjectMemberReference<typename TypeTraits<U>::type>(TypeTraits<U>::getPointer(iter->second), nullptr, itemTypeInfo);
			}
			i++;
		}
	}
	return nullptr;
}


template<typename U>
const std::string ContainerMemberReference<std::map<std::string, U> >::getValueAsString() const
{
	if (parentObject.getIsValid()) { return Tools::append(container.size(), "x", getTypeName(itemTypeInfo)); }
	else { return "invalid"; }
}


template <typename U>
inline const char* ContainerMemberReference<std::map<std::string, U> >::getCustomTypeName() const 
{ 
	return getTypeName(itemTypeInfo); 
}