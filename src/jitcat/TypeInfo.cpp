/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/TypeInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeRegistry.h"
#include "jitcat/VariableEnumerator.h"

#include <sstream>

using namespace jitcat;
using namespace jitcat::Reflection;


TypeInfo::TypeInfo(const char* typeName, TypeCaster* caster):
	typeName(typeName),
	caster(caster)
{
}


TypeInfo::~TypeInfo()
{
}


void TypeInfo::addDeserializedMember(TypeMemberInfo* memberInfo)
{
	std::string lowerCaseMemberName = Tools::toLowerCase(memberInfo->memberName);
	members.emplace(lowerCaseMemberName,  memberInfo);
}


void TypeInfo::addDeserializedMemberFunction(MemberFunctionInfo* memberFunction)
{
	std::string lowerCaseMemberFunctionName = Tools::toLowerCase(memberFunction->memberFunctionName);
	memberFunctions.emplace(lowerCaseMemberFunctionName, memberFunction);
}


CatGenericType TypeInfo::getType(const std::string& dotNotation) const
{
	std::vector<std::string> indirectionList;
	Tools::split(dotNotation, ".", indirectionList, false);
	return getType(indirectionList, 0);
}


CatGenericType TypeInfo::getType(const std::vector<std::string>& indirectionList, int offset) const
{
	int indirectionListSize = (int)indirectionList.size();
	if (indirectionListSize > 0)
	{
		std::map<std::string, std::unique_ptr<TypeMemberInfo>>::const_iterator iter = members.find(indirectionList[offset]);
		if (iter != members.end())
		{
			TypeMemberInfo* memberInfo = iter->second.get();
			if (memberInfo->catType.isBasicType())
			{
				if (offset == indirectionListSize - 1)
				{
					return memberInfo->catType;
				}
			}
			else if (memberInfo->catType.isContainerType())
			{
				if (indirectionListSize > offset + 1)
				{
					offset++;
					if ((memberInfo->catType.isVectorType()
							&& Tools::isNumber(indirectionList[offset]))
						|| memberInfo->catType.isMapType())
					{
						if (offset == indirectionListSize - 1)
						{
							return memberInfo->catType;
						}
						else if (indirectionListSize > offset + 1)
						{
							return memberInfo->catType.getContainerItemType().getObjectType()->getType(indirectionList, offset + 1);
						}
					}
				}
			}
			else if (memberInfo->catType.isObjectType())
			{
				if (indirectionListSize > offset + 1)
				{
					return memberInfo->catType.getObjectType()->getType(indirectionList, offset + 1);
				}
			}
		}
	}
	return CatGenericType::errorType;
}


TypeMemberInfo* TypeInfo::getMemberInfo(const std::string& identifier) const
{
	auto iter = members.find(Tools::toLowerCase(identifier));
	if (iter != members.end())
	{
		return iter->second.get();
	}
	else
	{
		return nullptr;
	}
}


MemberFunctionInfo* TypeInfo::getMemberFunctionInfo(const std::string& identifier) const
{
	auto iter = memberFunctions.find(Tools::toLowerCase(identifier));
	if (iter != memberFunctions.end())
	{
		return iter->second.get();
	}
	else
	{
		return nullptr;
	}
}


const char* TypeInfo::getTypeName() const
{
	return typeName;
}


void TypeInfo::setTypeName(const char* newTypeName)
{
	typeName = newTypeName;
}


void TypeInfo::enumerateVariables(VariableEnumerator* enumerator, bool allowEmptyStructs) const
{
	for (auto& iter : memberFunctions)
	{
		std::stringstream result;
		result << iter.second->memberFunctionName;
		result << "(";
		std::size_t numArguments = iter.second->getNumberOfArguments();
		for (std::size_t i = 0; i < numArguments; i++)
		{
			if (i > 0)
			{
				result << ", ";
			}
			result << iter.second->getArgumentType(i).toString();
		}
		result << ")";
		enumerator->addFunction(iter.second->memberFunctionName, result.str());
	}

	auto last = members.end();
	for (auto iter = members.begin(); iter != last; ++iter)
	{
		const CatGenericType& memberType = iter->second->catType;
		if (memberType.isBasicType())
		{
			std::string catTypeName = memberType.toString();
			enumerator->addVariable(iter->second->memberName, catTypeName, iter->second->catType.isWritable(), iter->second->catType.isConst());
			break;
		}
		else if (memberType.isObjectType())
		{
			std::string nestedTypeName = memberType.toString();
			if (allowEmptyStructs || memberType.getObjectType()->getMembers().size() > 0)
			{
				enumerator->enterNameSpace(iter->second->memberName, nestedTypeName, VariableEnumerator::NT_OBJECT);
				if (!Tools::isInList(enumerator->loopDetectionTypeStack, nestedTypeName))
				{
					enumerator->loopDetectionTypeStack.push_back(nestedTypeName);
					memberType.getObjectType()->enumerateVariables(enumerator, allowEmptyStructs);
					enumerator->loopDetectionTypeStack.pop_back();
				}
				enumerator->exitNameSpace();
					
			}
		}
		else if (memberType.isContainerType())
		{
			std::string containerType = memberType.isMapType() ? "Map" : "List";
			std::string itemType = memberType.getContainerItemType().toString();
			enumerator->enterNameSpace(iter->second->memberName, Tools::append(containerType, ": ", itemType), memberType.isMapType() ? VariableEnumerator::NT_MAP : VariableEnumerator::NT_VECTOR);
			if (!Tools::isInList(enumerator->loopDetectionTypeStack, itemType))
			{
				enumerator->loopDetectionTypeStack.push_back(itemType);
				memberType.getContainerItemType().getObjectType()->enumerateVariables(enumerator, allowEmptyStructs);
				enumerator->loopDetectionTypeStack.pop_back();
			}
			enumerator->exitNameSpace();
		}
	}
}


bool TypeInfo::isCustomType() const
{
	return false;
}


const std::map<std::string, std::unique_ptr<TypeMemberInfo>>& TypeInfo::getMembers() const
{
	return members;
}


const std::map<std::string, std::unique_ptr<MemberFunctionInfo>>& TypeInfo::getMemberFunctions() const
{
	return memberFunctions;
}


const TypeCaster* TypeInfo::getTypeCaster() const
{
	return caster.get();
}

