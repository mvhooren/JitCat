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
	TypeInfo(typeName, 0, new ObjectTypeCaster<CustomTypeInstance>()),
	defaultData(nullptr),
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


unsigned char* CustomTypeInfo::instanceConstructor() const
{
	unsigned char* data = new unsigned char[typeSize];
	createDataCopy(defaultData, typeSize, data, typeSize);
	return data;
}


void jitcat::Reflection::CustomTypeInfo::instanceConstructorInPlace(unsigned char* buffer, std::size_t bufferSize)
{
	createDataCopy(defaultData, typeSize, buffer, bufferSize);
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
	instanceDestructorInPlace(data);
	delete[] data;
}


void jitcat::Reflection::CustomTypeInfo::instanceDestructorInPlace(unsigned char* data)
{
	auto end = members.end();
	for (auto iter = members.begin(); iter != end; ++iter)
	{
		if (iter->second->isDeferred())
		{
			continue;
		}
		if (iter->second->catType.isStringType())
		{
			std::string* string;
			std::size_t offset = static_cast<CustomBasicTypeMemberInfo<std::string>*>(iter->second.get())->memberOffset;
			memcpy(&string, &data[offset], sizeof(std::string*));
			delete string;
		}
		else if (iter->second->catType.isReflectableHandleType())
		{
			CustomTypeObjectMemberInfo* memberInfo = static_cast<CustomTypeObjectMemberInfo*>(iter->second.get());
			std::size_t offset = memberInfo->memberOffset;
			ReflectableHandle* handle = reinterpret_cast<ReflectableHandle*>(data + offset);
			if (handle != nullptr)
			{
				if (iter->second->catType.getOwnershipSemantics() == TypeOwnershipSemantics::Owned)
				{
					memberInfo->catType.getPointeeType()->getObjectType()->destruct(handle->get());
				}
			}
			handle->~ReflectableHandle();
		}
	}
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


TypeMemberInfo* CustomTypeInfo::addObjectMember(const std::string& memberName, Reflectable* defaultValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics, bool isWritable, bool isConst)
{
	isTriviallyCopyable = false;
	std::size_t offset = 0;
	TypeMemberInfo* memberInfo = nullptr;
	if (ownershipSemantics != TypeOwnershipSemantics::Value)
	{
		CatGenericType type = CatGenericType(objectTypeInfo, isWritable, isConst).toHandle(ownershipSemantics, isWritable, isConst);
		offset = addReflectableHandle(defaultValue);
		memberInfo = new CustomTypeObjectMemberInfo(memberName, offset, CatGenericType(objectTypeInfo, isWritable, isConst).toHandle(ownershipSemantics, isWritable, isConst));
	}
	else
	{

	}

	
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	members.emplace(lowerCaseMemberName, memberInfo);
	
	if (Tools::startsWith(memberName, "$"))
	{
		addDeferredMembers(memberInfo);
	}
	
	return memberInfo;
}


TypeMemberInfo* jitcat::Reflection::CustomTypeInfo::addMember(const std::string& memberName, const CatGenericType& type)
{
	if		(type.isFloatType())						return addFloatMember(memberName, 0.0f, type.isWritable(), type.isConst());
	else if (type.isIntType())							return addIntMember(memberName, 0, type.isWritable(), type.isConst());
	else if (type.isBoolType())							return addBoolMember(memberName, false, type.isWritable(), type.isConst());
	else if (type.isStringType())						return addStringMember(memberName, "", type.isWritable(), type.isConst());
	else if (type.isPointerToReflectableObjectType())	return addObjectMember(memberName, nullptr, type.getPointeeType()->getObjectType(), type.getOwnershipSemantics(), type.isWritable(), type.isConst());
	else												return nullptr;
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
	if (iter != members.end() && !iter->second->isDeferred())
	{
		removedMembers.push_back(std::move(iter->second));
		members.erase(iter);
	}
}


void CustomTypeInfo::renameMember(const std::string& oldMemberName, const std::string& newMemberName)
{
	auto iter = members.find(Tools::toLowerCase(oldMemberName));
	if (iter != members.end() && members.find(Tools::toLowerCase(newMemberName)) == members.end() && !iter->second->isDeferred())
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


void jitcat::Reflection::CustomTypeInfo::construct(unsigned char* buffer, std::size_t bufferSize) const
{
	createDataCopy(defaultData, typeSize, buffer, bufferSize);
}


Reflectable* jitcat::Reflection::CustomTypeInfo::construct() const
{
	return reinterpret_cast<Reflectable*>(instanceConstructor());
}


void jitcat::Reflection::CustomTypeInfo::destruct(Reflectable* object)
{
	instanceDestructorInPlace((unsigned char*)object);
	Reflectable::destruct(object);

}


std::size_t jitcat::Reflection::CustomTypeInfo::addReflectableHandle(Reflectable* defaultValue)
{
	unsigned char* data = increaseDataSize(sizeof(ReflectableHandle));
	std::size_t offset = (data - defaultData);
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
	return offset;
}


std::size_t jitcat::Reflection::CustomTypeInfo::addObjectDataMember(TypeInfo* type)
{
	unsigned char* data = increaseDataSize(type->getTypeSize());
	std::size_t offset = (data - defaultData);
	//QQQ
	return offset;
}


unsigned char* CustomTypeInfo::increaseDataSize(std::size_t amount)
{
	std::size_t oldSize = typeSize;
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


void CustomTypeInfo::increaseDataSize(unsigned char*& data, std::size_t amount, std::size_t currentSize)
{
	unsigned char* oldData = data;
	std::size_t oldSize = currentSize;
	std::size_t newSize = oldSize + amount;

	if (oldData != nullptr
		&& oldSize != 0)
	{
		data = new unsigned char[newSize];
		createDataCopy(oldData, currentSize, data, newSize);
		instanceDestructor(oldData);
	}
	else
	{
		data = new unsigned char[newSize];
	}
	//Initialise the additional memory to zero
	memset(data + oldSize, 0, amount);
}


void CustomTypeInfo::createDataCopy(unsigned char* sourceData, std::size_t sourceSize, unsigned char* copyData, std::size_t copySize) const
{
	assert(copySize >= sourceSize);
	assert(sourceData != nullptr);
	assert(copyData != nullptr);
	//Create copies of strings and member references
	memcpy(copyData, sourceData, sourceSize);
	if (!isTriviallyCopyable)
	{
		auto end = members.end();
		for (auto iter = members.begin(); iter != end; ++iter)
		{
			if (iter->second->isDeferred())
			{
				continue;
			}
			if (iter->second->catType.isStringType())
			{
				std::string* originalString;
				std::size_t offset = static_cast<CustomBasicTypeMemberInfo<std::string>*>(iter->second.get())->memberOffset;
				memcpy(&originalString, &sourceData[offset], sizeof(std::string*));
				std::string* stringCopy = new std::string(*originalString);
				memcpy(copyData + offset, &stringCopy, sizeof(std::string*));
			}
			else if (iter->second->catType.isReflectableHandleType())
			{
				std::size_t offset = static_cast<CustomTypeObjectMemberInfo*>(iter->second.get())->memberOffset;
				ReflectableHandle* handle = reinterpret_cast<ReflectableHandle*>(&sourceData[offset]);
				new (copyData + offset) ReflectableHandle(handle->get());
			}
		}
	}
}
