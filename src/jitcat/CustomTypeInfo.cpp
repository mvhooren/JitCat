/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CustomTypeInfo.h"
#include "jitcat/Configuration.h"
#include "jitcat/ContainerManipulator.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/CustomTypeMemberFunctionInfo.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/StaticMemberInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeRegistry.h"

#include <cassert>
#include <iostream>

using namespace jitcat::Reflection;


CustomTypeInfo::CustomTypeInfo(const char* typeName, bool isConstType):
	TypeInfo(typeName, 0, new CustomObjectTypeCaster(this)),
	defaultData(nullptr),
	triviallyCopyable(true),
	isConstType(isConstType)
{
}


CustomTypeInfo::~CustomTypeInfo()
{
	ArrayManipulator::deleteArrayManipulatorsOfType(this);
	if (defaultData != nullptr)
	{
		instanceDestructor(defaultData);
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

	std::set<Reflectable*>::iterator end = instances.end();
	for (std::set<Reflectable*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((unsigned char*)(*iter) + offset, &defaultValue, sizeof(float));
	}

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<float>(memberName, offset, CatGenericType::createFloatType(isWritable, isConst));
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	TypeInfo::addMember(lowerCaseMemberName, memberInfo);
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

	std::set<Reflectable*>::iterator end = instances.end();
	for (std::set<Reflectable*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((unsigned char*)(*iter) + offset, &defaultValue, sizeof(int));
	}

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<int>(memberName, offset, CatGenericType::createIntType(isWritable, isConst));
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	TypeInfo::addMember(lowerCaseMemberName, memberInfo);
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

	std::set<Reflectable*>::iterator end = instances.end();
	for (std::set<Reflectable*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((unsigned char*)(*iter) + offset, &defaultValue, sizeof(bool));
	}

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<bool>(memberName, offset, CatGenericType::createBoolType(isWritable, isConst));
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	TypeInfo::addMember(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


TypeMemberInfo* CustomTypeInfo::addStringMember(const std::string& memberName, const std::string& defaultValue, bool isWritable, bool isConst)
{
	triviallyCopyable = false;
	unsigned char* data = increaseDataSize(sizeof(std::string));
	unsigned int offset = (unsigned int)(data - defaultData);
	if (defaultData == nullptr)
	{
		offset = 0;
	}

	std::set<Reflectable*>::iterator end = instances.end();
	for (std::set<Reflectable*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		new ((unsigned char*)(*iter) + offset) std::string(defaultValue);
	}

	new (data) std::string(defaultValue);

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<std::string>(memberName, offset, CatGenericType::createStringType(isWritable, isConst));
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	TypeInfo::addMember(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


TypeMemberInfo* CustomTypeInfo::addObjectMember(const std::string& memberName, unsigned char* defaultValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics, bool isWritable, bool isConst)
{
	triviallyCopyable = false;
	std::size_t offset = 0;
	TypeMemberInfo* memberInfo = nullptr;
	if (ownershipSemantics != TypeOwnershipSemantics::Value)
	{
		CatGenericType type = CatGenericType(objectTypeInfo, isWritable, isConst).toHandle(ownershipSemantics, isWritable, isConst);
		offset = addReflectableHandle(reinterpret_cast<Reflectable*>(defaultValue));
		objectTypeInfo->addDependentType(this);
		memberInfo = new CustomTypeObjectMemberInfo(memberName, offset, CatGenericType(objectTypeInfo, isWritable, isConst).toHandle(ownershipSemantics, isWritable, isConst));
		std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
		TypeInfo::addMember(lowerCaseMemberName, memberInfo);
	
		if (Tools::startsWith(memberName, "$"))
		{
			addDeferredMembers(memberInfo);
		}
	
		return memberInfo;
	}
	else
	{
		return addDataObjectMember(memberName, objectTypeInfo);
	}
}


TypeMemberInfo* jitcat::Reflection::CustomTypeInfo::addDataObjectMember(const std::string& memberName, TypeInfo* objectTypeInfo)
{
	if (objectTypeInfo != this)
	{
		triviallyCopyable = triviallyCopyable && objectTypeInfo->isTriviallyCopyable();
		unsigned char* data = increaseDataSize(objectTypeInfo->getTypeSize());
		unsigned int offset = (unsigned int)(data - defaultData);
		if (defaultData == nullptr)
		{
			offset = 0;
		}

		std::set<Reflectable*>::iterator end = instances.end();
		for (std::set<Reflectable*>::iterator iter = instances.begin(); iter != end; ++iter)
		{
			objectTypeInfo->placementConstruct((unsigned char*)(*iter) + offset, objectTypeInfo->getTypeSize());
		}
		objectTypeInfo->placementConstruct(data, objectTypeInfo->getTypeSize());
		TypeMemberInfo* memberInfo = new  CustomTypeObjectDataMemberInfo(memberName, offset, CatGenericType(CatGenericType(objectTypeInfo, true, false), TypeOwnershipSemantics::Value, false, false, false));
		std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
		TypeInfo::addMember(lowerCaseMemberName, memberInfo);
		objectTypeInfo->addDependentType(this);
		
		if (Tools::startsWith(memberName, "$"))
		{
			addDeferredMembers(memberInfo);
		}

		return memberInfo;
	}
	else
	{
		assert(false);
		return nullptr;
	}
}


TypeMemberInfo* jitcat::Reflection::CustomTypeInfo::addMember(const std::string& memberName, const CatGenericType& type)
{
	if		(type.isFloatType())						return addFloatMember(memberName, 0.0f, type.isWritable(), type.isConst());
	else if (type.isIntType())							return addIntMember(memberName, 0, type.isWritable(), type.isConst());
	else if (type.isBoolType())							return addBoolMember(memberName, false, type.isWritable(), type.isConst());
	else if (type.isStringType())						return addStringMember(memberName, "", type.isWritable(), type.isConst());
	else if (type.isPointerToReflectableObjectType())	return addObjectMember(memberName, nullptr, type.getPointeeType()->getObjectType(), type.getOwnershipSemantics(), type.isWritable(), type.isConst());
	else if (type.isReflectableObjectType())			return addDataObjectMember(memberName, type.getObjectType());
	else												return nullptr;
}


StaticMemberInfo* jitcat::Reflection::CustomTypeInfo::addStaticFloatMember(const std::string& memberName, float defaultValue, bool isWritable, bool isConst)
{
	constexpr std::size_t dataSize = sizeof(float);
	unsigned char* memberData = new unsigned char[dataSize];
	staticData.emplace_back(memberData);
	memcpy(memberData, &defaultValue, dataSize);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	StaticMemberInfo* memberInfo = new StaticBasicTypeMemberInfo<float>(memberName, reinterpret_cast<float*>(memberData), CatGenericType::floatType);
	staticMembers.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


StaticMemberInfo* jitcat::Reflection::CustomTypeInfo::addStaticIntMember(const std::string& memberName, int defaultValue, bool isWritable, bool isConst)
{
	constexpr std::size_t dataSize = sizeof(int);
	unsigned char* memberData = new unsigned char[dataSize];
	staticData.emplace_back(memberData);
	memcpy(memberData, &defaultValue, dataSize);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	StaticMemberInfo* memberInfo = new StaticBasicTypeMemberInfo<int>(memberName, reinterpret_cast<int*>(memberData), CatGenericType::intType);
	staticMembers.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


StaticMemberInfo* jitcat::Reflection::CustomTypeInfo::addStaticBoolMember(const std::string& memberName, bool defaultValue, bool isWritable, bool isConst)
{
	constexpr std::size_t dataSize = sizeof(bool);
	unsigned char* memberData = new unsigned char[dataSize];
	staticData.emplace_back(memberData);
	memcpy(memberData, &defaultValue, dataSize);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	StaticMemberInfo* memberInfo = new StaticBasicTypeMemberInfo<bool>(memberName, reinterpret_cast<bool*>(memberData), CatGenericType::boolType);
	staticMembers.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


StaticMemberInfo* jitcat::Reflection::CustomTypeInfo::addStaticStringMember(const std::string& memberName, const std::string& defaultValue, bool isWritable, bool isConst)
{
	constexpr std::size_t dataSize = sizeof(std::string);
	unsigned char* memberData = new unsigned char[dataSize];
	staticData.emplace_back(memberData);
	new (memberData) std::string(memberName);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	StaticMemberInfo* memberInfo = new StaticBasicTypeMemberInfo<std::string>(memberName, reinterpret_cast<std::string*>(memberData), CatGenericType::stringType);
	staticMembers.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


StaticMemberInfo* jitcat::Reflection::CustomTypeInfo::addStaticObjectMember(const std::string& memberName, unsigned char* defaultValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics, bool isWritable, bool isConst)
{
	if (ownershipSemantics != TypeOwnershipSemantics::Value)
	{
		CatGenericType type = CatGenericType(objectTypeInfo, isWritable, isConst).toHandle(ownershipSemantics, isWritable, isConst);
		constexpr std::size_t dataSize = sizeof(ReflectableHandle);
		unsigned char* memberData = new unsigned char[dataSize];		
		staticData.emplace_back(memberData);
		objectTypeInfo->addDependentType(this);
		ReflectableHandle handle(reinterpret_cast<Reflectable*>(defaultValue));
		type.copyConstruct(memberData, dataSize, reinterpret_cast<unsigned char*>(&handle), dataSize); 
		StaticMemberInfo* memberInfo = new StaticClassHandleMemberInfo(memberName, reinterpret_cast<ReflectableHandle*>(memberData), type);
		std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
		staticMembers.emplace(lowerCaseMemberName, memberInfo);
		return memberInfo;
	}
	else
	{
		return addStaticDataObjectMember(memberName, objectTypeInfo);
	}
}


StaticMemberInfo* jitcat::Reflection::CustomTypeInfo::addStaticDataObjectMember(const std::string& memberName, TypeInfo* objectTypeInfo)
{
	if (objectTypeInfo != this)
	{
		std::size_t dataSize = objectTypeInfo->getTypeSize();
		unsigned char* memberData = new unsigned char[dataSize];
		staticData.emplace_back(memberData);

		objectTypeInfo->placementConstruct(memberData, objectTypeInfo->getTypeSize());
		StaticMemberInfo* memberInfo = new StaticClassObjectMemberInfo(memberName, memberData, CatGenericType(CatGenericType(objectTypeInfo, false, false), TypeOwnershipSemantics::Value, false, false, false));

		std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
		staticMembers.emplace(lowerCaseMemberName, memberInfo);

		objectTypeInfo->addDependentType(this);

		return memberInfo;
	}
	else
	{
		assert(false);
		return nullptr;
	}
}


StaticMemberInfo* jitcat::Reflection::CustomTypeInfo::addStaticMember(const std::string& memberName, const CatGenericType& type)
{
	if		(type.isFloatType())						return addStaticFloatMember(memberName, 0.0f, type.isWritable(), type.isConst());
	else if (type.isIntType())							return addStaticIntMember(memberName, 0, type.isWritable(), type.isConst());
	else if (type.isBoolType())							return addStaticBoolMember(memberName, false, type.isWritable(), type.isConst());
	else if (type.isStringType())						return addStaticStringMember(memberName, "", type.isWritable(), type.isConst());
	else if (type.isPointerToReflectableObjectType())	return addStaticObjectMember(memberName, nullptr, type.getPointeeType()->getObjectType(), type.getOwnershipSemantics(), type.isWritable(), type.isConst());
	else if (type.isReflectableObjectType())			return addStaticDataObjectMember(memberName, type.getObjectType());
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
	TypeMemberInfo* memberInfo = releaseMember(memberName);
	if (memberInfo != nullptr)
	{
		removedMembers.emplace_back(memberInfo);
	}
}


Reflectable* CustomTypeInfo::getDefaultInstance()
{
	return reinterpret_cast<Reflectable*>(defaultData);
}


bool CustomTypeInfo::isCustomType() const
{
	return true;
}


void jitcat::Reflection::CustomTypeInfo::placementConstruct(unsigned char* buffer, std::size_t bufferSize) const
{
	auto& iter = instances.find(reinterpret_cast<Reflectable*>(buffer));
	if (iter == instances.end())
	{
		instances.insert(reinterpret_cast<Reflectable*>(buffer));
	}
	createDataCopy(defaultData, typeSize, buffer, bufferSize);
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		if (bufferSize > 0 && buffer != nullptr)
		{
			std::cout << "(CustomTypeInfo::placementConstruct) Placement constructed " << typeName << " at "<< std::hex << reinterpret_cast<uintptr_t>(buffer) << "\n";
		}
	}
}


void jitcat::Reflection::CustomTypeInfo::placementDestruct(unsigned char* buffer, std::size_t bufferSize)
{
	instanceDestructorInPlace(buffer);
	removeInstance(reinterpret_cast<Reflectable*>(buffer));
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		if (bufferSize > 0 && buffer != nullptr)
		{
			std::cout << "(CustomTypeInfo::placementDestruct) Placement destructed " << typeName << " at " << std::hex << reinterpret_cast<uintptr_t>(buffer) << "\n";
		}
	}
}


void jitcat::Reflection::CustomTypeInfo::copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	std::size_t typeSize = getTypeSize();
	assert(typeSize <= targetBufferSize && typeSize <= sourceBufferSize);
	createDataCopy(sourceBuffer, typeSize, targetBuffer, typeSize);
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		if (targetBufferSize > 0 && targetBuffer != nullptr)
		{
			std::cout << "(CustomTypeInfo::copyConstruct) Copy constructed " << typeName << " at " << std::hex << reinterpret_cast<uintptr_t>(targetBuffer) << " from " << std::hex << reinterpret_cast<uintptr_t>(sourceBuffer) << "\n";
		}
	}
}


void jitcat::Reflection::CustomTypeInfo::moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	std::size_t typeSize = getTypeSize();
	assert(targetBufferSize >= typeSize && sourceBufferSize >= typeSize);
	assert(sourceBuffer != nullptr || sourceBufferSize == 0);
	assert(targetBuffer != nullptr);
	if (triviallyCopyable)
	{
		memcpy(targetBuffer, sourceBuffer, typeSize);
	}
	else
	{
		auto end = membersByOrdinal.end();
		for (auto iter = membersByOrdinal.begin(); iter != end; ++iter)
		{
			if (iter->second->isDeferred())
			{
				continue;
			}
			std::size_t memberOffset = static_cast<CustomMemberInfo*>(iter->second)->memberOffset;
			iter->second->catType.moveConstruct(&targetBuffer[memberOffset], iter->second->catType.getTypeSize(), &sourceBuffer[memberOffset], iter->second->catType.getTypeSize());
		}
	}
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		if (targetBufferSize > 0 && targetBuffer != nullptr)
		{
			std::cout << "(CustomTypeInfo::moveConstruct) Move constructed " << typeName << " at " << std::hex << reinterpret_cast<uintptr_t>(targetBuffer) << " from " << std::hex << reinterpret_cast<uintptr_t>(sourceBuffer) << "\n";
		}
	}
}


bool jitcat::Reflection::CustomTypeInfo::isTriviallyCopyable() const
{
	return triviallyCopyable;
}


bool jitcat::Reflection::CustomTypeInfo::canBeDeleted() const
{
	return dependentTypes.size() == 0 && instances.size() == 0;
}


void CustomTypeInfo::instanceDestructor(unsigned char* data)
{
	instanceDestructorInPlace(data);
	delete[] data;
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		std::cout << "(CustomTypeInfo::instanceDestructor) deallocated buffer of size " << std::dec << typeSize << ": " << std::hex << reinterpret_cast<uintptr_t>(data) << "\n";
	}
}


void jitcat::Reflection::CustomTypeInfo::instanceDestructorInPlace(unsigned char* data)
{
	auto end = membersByOrdinal.end();
	for (auto iter = membersByOrdinal.begin(); iter != end; ++iter)
	{
		if (iter->second->isDeferred())
		{
			continue;
		}
		else
		{
			CustomMemberInfo* customMember = static_cast<CustomMemberInfo*>(iter->second);
			iter->second->catType.placementDestruct(&data[customMember->memberOffset], customMember->catType.getTypeSize());
		}
	}
	Reflectable::placementDestruct(reinterpret_cast<Reflectable*>(data));
}


std::size_t jitcat::Reflection::CustomTypeInfo::addReflectableHandle(Reflectable* defaultValue)
{
	unsigned char* data = increaseDataSize(sizeof(ReflectableHandle));
	std::size_t offset = (data - defaultData);
	if (defaultData == nullptr)
	{
		offset = 0;
	}

	std::set<Reflectable*>::iterator end = instances.end();
	for (std::set<Reflectable*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		new ((unsigned char*)(*iter) + offset) ReflectableHandle(defaultValue);
	}
	new (data) ReflectableHandle(defaultValue);
	return offset;
}


unsigned char* CustomTypeInfo::increaseDataSize(std::size_t amount)
{
	std::size_t oldSize = typeSize;
	increaseDataSize(defaultData, amount, typeSize);
	typeSize += amount;

	std::set<Reflectable*>::iterator end = instances.end();
	std::set<Reflectable*> replacedSet;
	for (std::set<Reflectable*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		unsigned char* data = (unsigned char*)(*iter);
		increaseDataSize(data, amount, oldSize);
		replacedSet.insert(reinterpret_cast<Reflectable*>(data));
	}
	instances = replacedSet;
	return defaultData + oldSize;
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
		if constexpr (Configuration::logJitCatObjectConstructionEvents)
		{
			std::cout << "(CustomTypeInfo::increaseDataSize) Allocated buffer of size " << std::dec << newSize << ": " << std::hex << reinterpret_cast<uintptr_t>(data) << "\n";
		}
		createDataCopy(oldData, currentSize, data, newSize);
		Reflectable::replaceReflectable(reinterpret_cast<Reflectable*>(oldData), reinterpret_cast<Reflectable*>(data));
		instanceDestructor(oldData);
	}
	else
	{
		data = new unsigned char[newSize];
		if constexpr (Configuration::logJitCatObjectConstructionEvents)
		{
			std::cout << "(CustomTypeInfo::increaseDataSize) Allocated buffer of size " << std::dec << newSize << ": " << std::hex << reinterpret_cast<uintptr_t>(data) << "\n";
		}
		Reflectable::replaceReflectable(reinterpret_cast<Reflectable*>(oldData), reinterpret_cast<Reflectable*>(data));
	}
	//Initialise the additional memory to zero
	memset(data + oldSize, 0, amount);
}


void CustomTypeInfo::createDataCopy(const unsigned char* sourceData, std::size_t sourceSize, unsigned char* copyData, std::size_t copySize) const
{
	assert(copySize >= sourceSize);
	assert(sourceData != nullptr || sourceSize == 0);
	assert(copyData != nullptr);
	//Create copies of strings and member references
	memcpy(copyData, sourceData, sourceSize);
	if (!triviallyCopyable)
	{
		auto end = membersByOrdinal.end();
		for (auto iter = membersByOrdinal.begin(); iter != end; ++iter)
		{
			if (iter->second->isDeferred())
			{
				continue;
			}
			std::size_t memberOffset = static_cast<CustomMemberInfo*>(iter->second)->memberOffset;
			iter->second->catType.copyConstruct(&copyData[memberOffset], iter->second->catType.getTypeSize(), &sourceData[memberOffset], iter->second->catType.getTypeSize());
		}
	}
	else
	{
		memcpy(copyData, sourceData, sourceSize);
	}
}


void jitcat::Reflection::CustomTypeInfo::removeInstance(Reflectable* instance)
{
	auto& iter = instances.find(instance);
	if (iter != instances.end())
	{
		instances.erase(iter);
	}
}
