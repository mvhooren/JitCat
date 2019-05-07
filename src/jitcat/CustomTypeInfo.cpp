/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/CustomTypeMemberFunctionInfo.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeRegistry.h"

#include <cassert>

using namespace jitcat::Reflection;


CustomTypeInfo::CustomTypeInfo(const char* typeName, bool isConstType):
	TypeInfo(typeName, new ObjectTypeCaster<CustomTypeInstance>()),
	defaultData(nullptr),
	typeSize(0),
	isTriviallyCopyable(true),
	isConstType(isConstType)
{
	defaultInstance = new CustomTypeInstance(defaultData, this);
}


CustomTypeInfo::~CustomTypeInfo()
{
	//Deleting this will also call instanceDestructor on data
	delete defaultInstance;
}


CustomTypeInstance* CustomTypeInfo::createInstance()
{
	if (!isConstType)
	{
		CustomTypeInstance* instance = new CustomTypeInstance(this);
		instances.insert(instance);
		return instance;
	}
	else
	{
		return nullptr;
	}
}


CustomTypeInstance* CustomTypeInfo::createInstanceCopy(CustomTypeInstance* source)
{
	if (!isConstType)
	{
		CustomTypeInstance* instance = new CustomTypeInstance(createDataCopy(source->data, typeSize), this);
		instances.insert(instance);
		return instance;
	}
	else
	{
		return nullptr;
	}
}


unsigned char* CustomTypeInfo::instanceConstructor()
{
	return createDataCopy(defaultData, typeSize);
}


void CustomTypeInfo::instanceDestructor(CustomTypeInstance* instance)
{
	std::set<CustomTypeInstance*>::iterator iter = instances.find(instance);
	if (iter != instances.end())
	{
		instances.erase(iter);
	}
	instanceDestructor(instance->data);
}


void CustomTypeInfo::instanceDestructor(unsigned char* data)
{
	auto end = members.end();
	for (auto iter = members.begin(); iter != end; ++iter)
	{
		if (iter->second->catType.isStringType())
		{
			std::string* string;
			unsigned int offset = static_cast<CustomBasicTypeMemberInfo<std::string>*>(iter->second.get())->memberOffset;
			memcpy(&string, &data[offset], sizeof(std::string*));
			delete string;
		}
		else if (iter->second->catType.isObjectType())
		{
			unsigned int offset = static_cast<CustomTypeObjectMemberInfo*>(iter->second.get())->memberOffset;
			ReflectableHandle* handle = reinterpret_cast<ReflectableHandle*>(data + offset);
			handle->~ReflectableHandle();
		}
	}
	delete[] data;
}


TypeMemberInfo* CustomTypeInfo::addFloatMember(const std::string& memberName, float defaultValue, bool isWritable, bool isConst)
{
	unsigned char* data = increaseDataSize(sizeof(float));
	memcpy(data, &defaultValue, sizeof(float));
	unsigned int offset = (unsigned int)(data - defaultData);
	if (defaultData == nullptr)
	{
		offset = 0;
	}

	std::set<CustomTypeInstance*>::iterator end = instances.end();
	for (std::set<CustomTypeInstance*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((*iter)->data + offset, &defaultValue, sizeof(float));
	}

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<float>(memberName, offset, CatGenericType::createFloatType(isWritable, isConst));
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	members.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


TypeMemberInfo* CustomTypeInfo::addIntMember(const std::string& memberName, int defaultValue, bool isWritable, bool isConst)
{
	unsigned char* data = increaseDataSize(sizeof(int));
	memcpy(data, &defaultValue, sizeof(int));
	unsigned int offset = (unsigned int)(data - defaultData);
	if (defaultData == nullptr)
	{
		offset = 0;
	}

	std::set<CustomTypeInstance*>::iterator end = instances.end();
	for (std::set<CustomTypeInstance*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((*iter)->data + offset, &defaultValue, sizeof(int));
	}

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<int>(memberName, offset, CatGenericType::createIntType(isWritable, isConst));
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	members.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


TypeMemberInfo* CustomTypeInfo::addBoolMember(const std::string& memberName, bool defaultValue, bool isWritable, bool isConst)
{
	unsigned char* data = increaseDataSize(sizeof(bool));
	memcpy(data, &defaultValue, sizeof(bool));
	unsigned int offset = (unsigned int)(data - defaultData);
	if (defaultData == nullptr)
	{
		offset = 0;
	}

	std::set<CustomTypeInstance*>::iterator end = instances.end();
	for (std::set<CustomTypeInstance*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((*iter)->data + offset, &defaultValue, sizeof(bool));
	}

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<bool>(memberName, offset, CatGenericType::createBoolType(isWritable, isConst));
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	members.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


TypeMemberInfo* CustomTypeInfo::addStringMember(const std::string& memberName, const std::string& defaultValue, bool isWritable, bool isConst)
{
	isTriviallyCopyable = false;
	unsigned char* data = increaseDataSize(sizeof(std::string*));
	unsigned int offset = (unsigned int)(data - defaultData);
	if (defaultData == nullptr)
	{
		offset = 0;
	}

	std::set<CustomTypeInstance*>::iterator end = instances.end();
	for (std::set<CustomTypeInstance*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		std::string* newString = new std::string(defaultValue);
		memcpy((*iter)->data + offset, &newString, sizeof(std::string*));
	}

	std::string* newString = new std::string(defaultValue);
	memcpy(data, &newString, sizeof(std::string*));
	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<std::string>(memberName, offset, CatGenericType::createStringType(isWritable, isConst));
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	members.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


TypeMemberInfo* CustomTypeInfo::addObjectMember(const std::string& memberName, Reflectable* defaultValue, TypeInfo* objectTypeInfo, bool isWritable, bool isConst)
{
	if (defaultValue != nullptr)
	{
		isTriviallyCopyable = false;
		unsigned char* data = increaseDataSize(sizeof(ReflectableHandle));
		unsigned int offset = (unsigned int)(data - defaultData);
		if (defaultData == nullptr)
		{
			offset = 0;
		}

		std::set<CustomTypeInstance*>::iterator end = instances.end();
		for (std::set<CustomTypeInstance*>::iterator iter = instances.begin(); iter != end; ++iter)
		{
			new ((*iter)->data + offset) ReflectableHandle(defaultValue);
		}
		new (data) ReflectableHandle(defaultValue);
		TypeMemberInfo* memberInfo = new CustomTypeObjectMemberInfo(memberName, offset, CatGenericType(objectTypeInfo, isWritable, isConst));
		std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
		members.emplace(lowerCaseMemberName, memberInfo);
		return memberInfo;
	}
	return nullptr;
}


TypeMemberInfo* jitcat::Reflection::CustomTypeInfo::addMember(const std::string& memberName, const CatGenericType& type)
{
	if		(type.isFloatType())	return addFloatMember(memberName, 0.0f, type.isWritable(), type.isConst());
	else if (type.isIntType())		return addIntMember(memberName, 0, type.isWritable(), type.isConst());
	else if (type.isBoolType())		return addBoolMember(memberName, false, type.isWritable(), type.isConst());
	else if (type.isStringType())	return addStringMember(memberName, "", type.isWritable(), type.isConst());
	else if (type.isObjectType())	return addObjectMember(memberName, nullptr, type.getObjectType(), type.isWritable(), type.isConst());
	else							return nullptr;
}


CustomTypeMemberFunctionInfo* jitcat::Reflection::CustomTypeInfo::addMemberFunction(const std::string& memberFunctionName, const CatGenericType& thisType, AST::CatFunctionDefinition* functionDefinition)
{
	CustomTypeMemberFunctionInfo* functionInfo = new CustomTypeMemberFunctionInfo(functionDefinition, thisType);
	memberFunctions.emplace(Tools::toLowerCase(memberFunctionName), functionInfo);
	return functionInfo;
}


void CustomTypeInfo::removeMember(const std::string& memberName)
{
	auto iter = members.find(Tools::toLowerCase(memberName));
	if (iter != members.end())
	{
		removedMembers.push_back(std::move(iter->second));
		members.erase(iter);
	}
}


void CustomTypeInfo::renameMember(const std::string& oldMemberName, const std::string& newMemberName)
{
	auto iter = members.find(Tools::toLowerCase(oldMemberName));
	if (iter != members.end() && members.find(Tools::toLowerCase(newMemberName)) == members.end())
	{
		std::unique_ptr<TypeMemberInfo> memberInfo = std::move(iter->second);
		memberInfo->memberName = newMemberName;
		members.erase(iter);
		std::string lowerCaseMemberName = Tools::toLowerCase(newMemberName);
		members.emplace(lowerCaseMemberName, std::move(memberInfo));
	}
}


CustomTypeInstance* CustomTypeInfo::getDefaultInstance()
{
	return defaultInstance;
}


bool CustomTypeInfo::isCustomType() const
{
	return true;
}


unsigned char* CustomTypeInfo::increaseDataSize(unsigned int amount)
{
	unsigned int oldSize = typeSize;
	increaseDataSize(defaultData, amount, typeSize);
	defaultInstance->data = defaultData;
	typeSize += amount;

	std::set<CustomTypeInstance*>::iterator end = instances.end();
	for (std::set<CustomTypeInstance*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		increaseDataSize((*iter)->data, amount, oldSize);
	}

	return defaultInstance->data + oldSize;
}


void CustomTypeInfo::increaseDataSize(unsigned char*& data, unsigned int amount, unsigned int currentSize)
{
	unsigned char* oldData = data;
	unsigned int oldSize = currentSize;
	unsigned int newSize = oldSize + amount;

	if (oldData != nullptr
		&& oldSize != 0)
	{
		data = createDataCopy(oldData, newSize);
		instanceDestructor(oldData);
	}
	else
	{
		data = new unsigned char[newSize];
	}
	//Initialise the additional memory to zero
	memset(data + oldSize, 0, amount);
}


unsigned char* CustomTypeInfo::createDataCopy(unsigned char* otherData, unsigned int sizeOfCopy)
{
	assert(sizeOfCopy >= typeSize);
	//Create copies of strings and member references
	if (otherData != nullptr)
	{
		unsigned char* instanceData = new unsigned char[sizeOfCopy];
		memcpy(instanceData, otherData, typeSize);
		if (!isTriviallyCopyable)
		{
			auto end = members.end();
			for (auto iter = members.begin(); iter != end; ++iter)
			{
				if (iter->second->catType.isStringType())
				{
					std::string* originalString;
					unsigned int offset = static_cast<CustomBasicTypeMemberInfo<std::string>*>(iter->second.get())->memberOffset;
					memcpy(&originalString, &otherData[offset], sizeof(std::string*));
					std::string* stringCopy = new std::string(*originalString);
					memcpy(instanceData + offset, &stringCopy, sizeof(std::string*));
				}
				else if (iter->second->catType.isObjectType())
				{
					unsigned int offset = static_cast<CustomTypeObjectMemberInfo*>(iter->second.get())->memberOffset;
					ReflectableHandle* handle = reinterpret_cast<ReflectableHandle*>(&otherData[offset]);
					new (instanceData + offset) ReflectableHandle(handle->get());
				}
			}
		}
		return instanceData;
	}
	else
	{
		return nullptr;
	}
}
