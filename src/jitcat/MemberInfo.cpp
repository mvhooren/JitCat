/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "MemberInfo.h"

#include <sstream>


CatGenericType TypeMemberInfo::toGenericType() const
{
	switch (specificType)
	{
		case SpecificMemberType::CatType:			return CatGenericType(catType);
		case SpecificMemberType::NestedType:		return CatGenericType(nestedType);
		case SpecificMemberType::ContainerType:		return CatGenericType(containerType, nestedType);
		default:									return CatGenericType("Error");
	}

}


std::string TypeMemberInfo::getFullTypeName() const
{
	std::stringstream nameStream;
	switch (specificType)
	{
		case SpecificMemberType::CatType:
			nameStream << toString(catType);
			break;
		case SpecificMemberType::NestedType:
			nameStream << nestedType->getTypeName();
			break;
		case SpecificMemberType::ContainerType:
			if (containerType == ContainerType::Vector)
			{
				nameStream << "list of " << nestedType->getTypeName();
			}
			else if (containerType == ContainerType::StringMap)
			{
				nameStream << "map of " << nestedType->getTypeName();
			}
			break;
	}
	if (isWritable)
	{
		nameStream << " (Writable)";
	}
	else if (isConst)
	{
		nameStream << " (Const)";
	}
	return nameStream.str();
}
