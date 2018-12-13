/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "TypeInfo.h"
#include "Tools.h"
#include "TypeRegistry.h"
#include "VariableEnumerator.h"

#include <sstream>


TypeInfo::TypeInfo(const char* typeName):
	typeName(typeName)
{
}


TypeInfo::~TypeInfo()
{
	Tools::deleteSecondElementsAndClear(members);
}


void TypeInfo::addDeserializedMember(TypeMemberInfo* memberInfo)
{
	std::string lowerCaseMemberName = Tools::toLowerCase(memberInfo->memberName);
	members[lowerCaseMemberName] = memberInfo;
}


void TypeInfo::addDeserializedMemberFunction(MemberFunctionInfo* memberFunction)
{
	std::string lowerCaseMemberFunctionName = Tools::toLowerCase(memberFunction->memberFunctionName);
	memberFunctions[lowerCaseMemberFunctionName] = memberFunction;
}


CatType TypeInfo::getType(const std::string& dotNotation) const
{
	std::vector<std::string> indirectionList;
	Tools::split(dotNotation, ".", indirectionList, false);
	return getType(indirectionList, 0);
}


CatType TypeInfo::getType(const std::vector<std::string>& indirectionList, int offset) const
{
	int indirectionListSize = (int)indirectionList.size();
	if (indirectionListSize > 0)
	{
		std::map<std::string, TypeMemberInfo*>::const_iterator iter = members.find(indirectionList[offset]);
		if (iter != members.end())
		{
			TypeMemberInfo* memberInfo = iter->second;
			switch (memberInfo->specificType)
			{
				case SpecificMemberType::CatType:
				{
					if (offset == indirectionListSize - 1)
					{
						return memberInfo->catType;
					}
					break;
				}
				case SpecificMemberType::ContainerType:
				{
					if (indirectionListSize > offset + 1)
					{
						offset++;
						if ((memberInfo->containerType == ContainerType::Vector
							 && Tools::isNumber(indirectionList[offset]))
						    || memberInfo->containerType == ContainerType::StringMap)
						{
							if (memberInfo->catType != CatType::Unknown
								&& offset == indirectionListSize - 1)
							{
								return memberInfo->catType;
							}
							else if (memberInfo->nestedType != nullptr
									 && indirectionListSize > offset + 1)
							{
								return memberInfo->nestedType->getType(indirectionList, offset + 1);
							}
						}
					}
					break;
				}
				break;
				case SpecificMemberType::NestedType:
				{
					if (indirectionListSize > offset + 1)
					{
						return memberInfo->nestedType->getType(indirectionList, offset + 1);
					}
					break;
				}
			}
		}
	}
	return CatType::Error;
}


TypeMemberInfo* TypeInfo::getMemberInfo(const std::string& identifier) const
{
	std::map<std::string, TypeMemberInfo*>::const_iterator iter = members.find(Tools::toLowerCase(identifier));
	if (iter != members.end())
	{
		return iter->second;
	}
	else
	{
		return nullptr;
	}
}


MemberFunctionInfo* TypeInfo::getMemberFunctionInfo(const std::string& identifier) const
{
	std::map<std::string, MemberFunctionInfo*>::const_iterator iter = memberFunctions.find(Tools::toLowerCase(identifier));
	if (iter != memberFunctions.end())
	{
		return iter->second;
	}
	else
	{
		return nullptr;
	}
}


MemberReferencePtr TypeInfo::getMemberReference(MemberReferencePtr& derefBase, const std::string& identifier)
{
	std::map<std::string, TypeMemberInfo*>::iterator iter = members.find(Tools::toLowerCase(identifier));
	if (iter != members.end())
	{
		TypeMemberInfo* member = iter->second;
		return MemberReferencePtr(member->getMemberReference(derefBase));
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

	std::map<std::string, TypeMemberInfo*>::const_iterator last = members.end();
	for (std::map<std::string, TypeMemberInfo*>::const_iterator iter = members.begin(); iter != last; ++iter)
	{
		switch(iter->second->specificType)
		{
			case SpecificMemberType::CatType:
			{
				std::string catTypeName = toString(iter->second->catType);
				enumerator->addVariable(iter->second->memberName, catTypeName, iter->second->isWritable, iter->second->isConst);
				break;
			}
			case SpecificMemberType::NestedType:
			{
				std::string nestedTypeName = iter->second->nestedType->getTypeName();
				if (allowEmptyStructs || iter->second->nestedType->getMembers().size() > 0)
				{
					enumerator->enterNameSpace(iter->second->memberName, nestedTypeName, VariableEnumerator::NT_OBJECT);
					if (!Tools::isInList(enumerator->loopDetectionTypeStack, nestedTypeName))
					{
						enumerator->loopDetectionTypeStack.push_back(nestedTypeName);
						iter->second->nestedType->enumerateVariables(enumerator, allowEmptyStructs);
						enumerator->loopDetectionTypeStack.pop_back();
					}
					enumerator->exitNameSpace();
					
				}
				break;
			}
			case SpecificMemberType::ContainerType:
			{
				std::string containerType = iter->second->containerType == ContainerType::StringMap ? "Map" : "List";
				std::string itemType = iter->second->nestedType->getTypeName();
				enumerator->enterNameSpace(iter->second->memberName, Tools::append(containerType, ": ", itemType), iter->second->containerType == ContainerType::StringMap ? VariableEnumerator::NT_MAP : VariableEnumerator::NT_VECTOR);
				if (!Tools::isInList(enumerator->loopDetectionTypeStack, itemType))
				{
					enumerator->loopDetectionTypeStack.push_back(itemType);
					iter->second->nestedType->enumerateVariables(enumerator, allowEmptyStructs);
					enumerator->loopDetectionTypeStack.pop_back();
				}
				enumerator->exitNameSpace();
				break;
			}
		}
	}
}


bool TypeInfo::isCustomType() const
{
	return false;
}


const std::map<std::string, TypeMemberInfo*>& TypeInfo::getMembers() const
{
	return members;
}


const std::map<std::string, MemberFunctionInfo*>& TypeInfo::getMemberFunctions() const
{
	return memberFunctions;
}
