/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/Configuration.h"
#include "jitcat/CustomObject.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/CustomTypeMemberFunctionInfo.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/StaticMemberInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeRegistry.h"

#include <cassert>
#include <iostream>

using namespace jitcat;
using namespace jitcat::Reflection;


CustomTypeInfo::CustomTypeInfo(const char* typeName, HandleTrackingMethod trackingMethod):
	TypeInfo(typeName, 0, std::make_unique<CustomObjectTypeCaster>(this)),
	trackingMethod(trackingMethod),
	classDefinition(nullptr),
	defaultData(new unsigned char[0]),
	triviallyCopyable(true),
	triviallyConstructable(true),
	defaultConstructorFunction(nullptr),
	destructorFunction(nullptr),
	dylib(nullptr)
{
	if (trackingMethod == HandleTrackingMethod::InternalHandlePointer)
	{
		addObjectMember<ReflectableHandle*>("_firstHandle", nullptr, TypeRegistry::get()->registerType<ReflectableHandle>());
	}
}


CustomTypeInfo::CustomTypeInfo(AST::CatClassDefinition* classDefinition, HandleTrackingMethod trackingMethod):
	TypeInfo(classDefinition->getClassName().c_str(), 0, std::make_unique<CustomObjectTypeCaster>(this)),
	trackingMethod(trackingMethod),
	classDefinition(classDefinition),
	defaultData(new unsigned char[0]),
	triviallyCopyable(true),
	triviallyConstructable(true),
	defaultConstructorFunction(nullptr),
	destructorFunction(nullptr),
	dylib(nullptr)
{
	if (trackingMethod == HandleTrackingMethod::InternalHandlePointer)
	{
		addObjectMember<ReflectableHandle*>("_firstHandle", nullptr, TypeRegistry::get()->registerType<ReflectableHandle>());
	}
}


CustomTypeInfo::~CustomTypeInfo()
{
	if (defaultData != nullptr)
	{
		placementDestruct(defaultData, getTypeSize());
		delete[] defaultData;
	}
}


TypeMemberInfo* CustomTypeInfo::addDoubleMember(const std::string& memberName, float defaultValue, bool isWritable, bool isConst)
{
	unsigned char* data = increaseDataSize(sizeof(double));
	memcpy(data, &defaultValue, sizeof(double));
	unsigned int offset = (unsigned int)(data - defaultData);
	if (defaultData == nullptr)
	{
		offset = 0;
	}

	std::set<unsigned char*>::iterator end = instances.end();
	for (std::set<unsigned char*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((unsigned char*)(*iter) + offset, &defaultValue, sizeof(double));
	}

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<double>(memberName, offset, CatGenericType::createDoubleType(isWritable, isConst), getTypeName());
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	TypeInfo::addMember(lowerCaseMemberName, memberInfo);
	return memberInfo;
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

	std::set<unsigned char*>::iterator end = instances.end();
	for (std::set<unsigned char*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((unsigned char*)(*iter) + offset, &defaultValue, sizeof(float));
	}

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<float>(memberName, offset, CatGenericType::createFloatType(isWritable, isConst), getTypeName());
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

	std::set<unsigned char*>::iterator end = instances.end();
	for (std::set<unsigned char*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((*iter) + offset, &defaultValue, sizeof(int));
	}

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<int>(memberName, offset, CatGenericType::createIntType(isWritable, isConst), getTypeName());
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

	std::set<unsigned char*>::iterator end = instances.end();
	for (std::set<unsigned char*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((*iter) + offset, &defaultValue, sizeof(bool));
	}

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<bool>(memberName, offset, CatGenericType::createBoolType(isWritable, isConst), getTypeName());
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	TypeInfo::addMember(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


TypeMemberInfo* CustomTypeInfo::addVector4fMember(const std::string& memberName, const std::array<float, 4> defaultValue, bool isWritable, bool isConst)
{
	unsigned char* data = increaseDataSize(sizeof(defaultValue));
	memcpy(data, &defaultValue, sizeof(defaultValue));
	unsigned int offset = (unsigned int)(data - defaultData);
	if (defaultData == nullptr)
	{
		offset = 0;
	}

	std::set<unsigned char*>::iterator end = instances.end();
	for (std::set<unsigned char*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		memcpy((*iter) + offset, &defaultValue, sizeof(defaultValue));
	}
	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<std::array<float, 4>>(memberName, offset, CatGenericType::createVector4fType(isWritable, isConst), getTypeName());
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	TypeInfo::addMember(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


TypeMemberInfo* CustomTypeInfo::addStringMember(const std::string& memberName, const Configuration::CatString& defaultValue, bool isWritable, bool isConst)
{
	triviallyCopyable = false;
	triviallyConstructable = false;
	unsigned char* data = increaseDataSize(sizeof(Configuration::CatString));
	unsigned int offset = (unsigned int)(data - defaultData);
	if (defaultData == nullptr)
	{
		offset = 0;
	}

	std::set<unsigned char*>::iterator end = instances.end();
	for (std::set<unsigned char*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		new ((*iter) + offset) Configuration::CatString(defaultValue);
	}

	new (data) Configuration::CatString(defaultValue);

	TypeMemberInfo* memberInfo = new CustomTypeObjectDataMemberInfo(memberName, offset, CatGenericType::createStringType(isWritable, isConst), getTypeName());
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	TypeInfo::addMember(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


TypeMemberInfo* CustomTypeInfo::addObjectMember(const std::string& memberName, unsigned char* defaultValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics, bool isWritable, bool isConst)
{
	std::size_t offset = 0;
	TypeMemberInfo* memberInfo = nullptr;
	if (ownershipSemantics != TypeOwnershipSemantics::Value)
	{
		triviallyCopyable = false;
		triviallyConstructable = false;
		CatGenericType type = CatGenericType(objectTypeInfo, isWritable, isConst).toHandle(ownershipSemantics, isWritable, isConst);
		offset = addReflectableHandle(defaultValue, objectTypeInfo);
		objectTypeInfo->addDependentType(this);
		memberInfo = new CustomTypeObjectMemberInfo(memberName, offset, CatGenericType(objectTypeInfo, isWritable, isConst).toHandle(ownershipSemantics, isWritable, isConst), getTypeName());
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


TypeMemberInfo* CustomTypeInfo::addDataObjectMember(const std::string& memberName, TypeInfo* objectTypeInfo)
{
	if (objectTypeInfo != this)
	{
		triviallyCopyable = triviallyCopyable && objectTypeInfo->isTriviallyCopyable();
		triviallyConstructable = triviallyConstructable && objectTypeInfo->isTriviallyConstructable();
		unsigned char* data = increaseDataSize(objectTypeInfo->getTypeSize());
		unsigned int offset = (unsigned int)(data - defaultData);
		if (defaultData == nullptr)
		{
			offset = 0;
		}

		std::set<unsigned char*>::iterator end = instances.end();
		for (std::set<unsigned char*>::iterator iter = instances.begin(); iter != end; ++iter)
		{
			objectTypeInfo->placementConstruct((unsigned char*)(*iter) + offset, objectTypeInfo->getTypeSize());
		}
		objectTypeInfo->placementConstruct(data, objectTypeInfo->getTypeSize());
		TypeMemberInfo* memberInfo = new  CustomTypeObjectDataMemberInfo(memberName, offset, CatGenericType(CatGenericType(objectTypeInfo, true, false), 
																		 TypeOwnershipSemantics::Value, false, false, false), getTypeName());
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


TypeMemberInfo* CustomTypeInfo::addMember(const std::string& memberName, const CatGenericType& type)
{
	if		(type.isFloatType())						return addFloatMember(memberName, 0.0f, type.isWritable(), type.isConst());
	else if	(type.isDoubleType())						return addDoubleMember(memberName, 0.0, type.isWritable(), type.isConst());
	else if (type.isIntType())							return addIntMember(memberName, 0, type.isWritable(), type.isConst());
	else if (type.isBoolType())							return addBoolMember(memberName, false, type.isWritable(), type.isConst());
	else if (type.isStringValueType())					return addStringMember(memberName, "", type.isWritable(), type.isConst());
	else if (type.isPointerToReflectableObjectType())	return addObjectMember(memberName, nullptr, type.getPointeeType()->getObjectType(), type.getOwnershipSemantics(), type.isWritable(), type.isConst());
	else if (type.isReflectableObjectType())			return addDataObjectMember(memberName, type.getObjectType());
	else												return nullptr;
}


StaticMemberInfo* CustomTypeInfo::addStaticDoubleMember(const std::string& memberName, double defaultValue, bool isWritable, bool isConst)
{
	constexpr std::size_t dataSize = sizeof(double);
	unsigned char* memberData = new unsigned char[dataSize];
	staticData.emplace_back(memberData);
	memcpy(memberData, &defaultValue, dataSize);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	StaticMemberInfo* memberInfo = new StaticBasicTypeMemberInfo<double>(memberName, reinterpret_cast<double*>(memberData), CatGenericType::doubleType, getTypeName());
	staticMembers.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


StaticMemberInfo* CustomTypeInfo::addStaticFloatMember(const std::string& memberName, float defaultValue, bool isWritable, bool isConst)
{
	constexpr std::size_t dataSize = sizeof(float);
	unsigned char* memberData = new unsigned char[dataSize];
	staticData.emplace_back(memberData);
	memcpy(memberData, &defaultValue, dataSize);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	StaticMemberInfo* memberInfo = new StaticBasicTypeMemberInfo<float>(memberName, reinterpret_cast<float*>(memberData), CatGenericType::floatType, getTypeName());
	staticMembers.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


StaticMemberInfo* CustomTypeInfo::addStaticIntMember(const std::string& memberName, int defaultValue, bool isWritable, bool isConst)
{
	constexpr std::size_t dataSize = sizeof(int);
	unsigned char* memberData = new unsigned char[dataSize];
	staticData.emplace_back(memberData);
	memcpy(memberData, &defaultValue, dataSize);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	StaticMemberInfo* memberInfo = new StaticBasicTypeMemberInfo<int>(memberName, reinterpret_cast<int*>(memberData), CatGenericType::intType, getTypeName());
	staticMembers.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


StaticMemberInfo* CustomTypeInfo::addStaticBoolMember(const std::string& memberName, bool defaultValue, bool isWritable, bool isConst)
{
	constexpr std::size_t dataSize = sizeof(bool);
	unsigned char* memberData = new unsigned char[dataSize];
	staticData.emplace_back(memberData);
	memcpy(memberData, &defaultValue, dataSize);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	StaticMemberInfo* memberInfo = new StaticBasicTypeMemberInfo<bool>(memberName, reinterpret_cast<bool*>(memberData), CatGenericType::boolType, getTypeName());
	staticMembers.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


StaticMemberInfo* CustomTypeInfo::addStaticStringMember(const std::string& memberName, const Configuration::CatString& defaultValue, bool isWritable, bool isConst)
{
	constexpr std::size_t dataSize = sizeof(Configuration::CatString);
	unsigned char* memberData = new unsigned char[dataSize];
	staticData.emplace_back(memberData);
	new (memberData) Configuration::CatString(defaultValue);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	StaticMemberInfo* memberInfo = new StaticClassObjectMemberInfo(memberName, memberData, CatGenericType::createStringType(isWritable, isConst), getTypeName());
	staticMembers.emplace(lowerCaseMemberName, memberInfo);
	return memberInfo;
}


StaticMemberInfo* CustomTypeInfo::addStaticObjectMember(const std::string& memberName, unsigned char* defaultValue, TypeInfo* objectTypeInfo, TypeOwnershipSemantics ownershipSemantics, bool isWritable, bool isConst)
{
	if (ownershipSemantics != TypeOwnershipSemantics::Value)
	{
		CatGenericType type = CatGenericType(objectTypeInfo, isWritable, isConst).toHandle(ownershipSemantics, isWritable, isConst);
		constexpr std::size_t dataSize = sizeof(ReflectableHandle);
		unsigned char* memberData = new unsigned char[dataSize];		
		staticData.emplace_back(memberData);
		objectTypeInfo->addDependentType(this);
		ReflectableHandle handle(defaultValue, objectTypeInfo);
		type.copyConstruct(memberData, dataSize, reinterpret_cast<unsigned char*>(&handle), dataSize); 
		StaticMemberInfo* memberInfo = new StaticClassHandleMemberInfo(memberName, reinterpret_cast<ReflectableHandle*>(memberData), type, getTypeName());
		std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
		staticMembers.emplace(lowerCaseMemberName, memberInfo);
		return memberInfo;
	}
	else
	{
		return addStaticDataObjectMember(memberName, objectTypeInfo);
	}
}


StaticMemberInfo* CustomTypeInfo::addStaticDataObjectMember(const std::string& memberName, TypeInfo* objectTypeInfo)
{
	if (objectTypeInfo != this)
	{
		std::size_t dataSize = objectTypeInfo->getTypeSize();
		unsigned char* memberData = new unsigned char[dataSize];
		staticData.emplace_back(memberData);

		objectTypeInfo->placementConstruct(memberData, objectTypeInfo->getTypeSize());
		StaticMemberInfo* memberInfo = new StaticClassObjectMemberInfo(memberName, memberData, CatGenericType(CatGenericType(objectTypeInfo, false, false), 
																	   TypeOwnershipSemantics::Value, false, false, false), getTypeName());

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


StaticMemberInfo* CustomTypeInfo::addStaticMember(const std::string& memberName, const CatGenericType& type)
{
	if		(type.isFloatType())						return addStaticFloatMember(memberName, 0.0f, type.isWritable(), type.isConst());
	else if (type.isDoubleType())						return addStaticDoubleMember(memberName, 0.0, type.isWritable(), type.isConst());
	else if (type.isIntType())							return addStaticIntMember(memberName, 0, type.isWritable(), type.isConst());
	else if (type.isBoolType())							return addStaticBoolMember(memberName, false, type.isWritable(), type.isConst());
	else if (type.isStringValueType())					return addStaticStringMember(memberName, "", type.isWritable(), type.isConst());
	else if (type.isPointerToReflectableObjectType())	return addStaticObjectMember(memberName, nullptr, type.getPointeeType()->getObjectType(), type.getOwnershipSemantics(), type.isWritable(), type.isConst());
	else if (type.isReflectableObjectType())			return addStaticDataObjectMember(memberName, type.getObjectType());
	else												return nullptr;
}


CustomTypeMemberFunctionInfo* CustomTypeInfo::addMemberFunction(const std::string& memberFunctionName, const CatGenericType& thisType, AST::CatFunctionDefinition* functionDefinition)
{
	CustomTypeMemberFunctionInfo* functionInfo = new CustomTypeMemberFunctionInfo(functionDefinition, thisType);
	memberFunctions.emplace(Tools::toLowerCase(memberFunctionName), functionInfo);
	return functionInfo;
}


bool CustomTypeInfo::setDefaultConstructorFunction(const std::string& constructorFunctionName)
{
	SearchFunctionSignature sig(constructorFunctionName, {});
	MemberFunctionInfo* functionInfo = getMemberFunctionInfo(sig);
	if (functionInfo != nullptr)
	{
		defaultConstructorFunction = functionInfo;
		return true;
	}
	return false;
}


bool CustomTypeInfo::setDestructorFunction(const std::string& destructorFunctionName)
{
	SearchFunctionSignature sig(destructorFunctionName, {});
	MemberFunctionInfo* functionInfo = getMemberFunctionInfo(sig);
	if (functionInfo != nullptr)
	{
		destructorFunction = functionInfo;
		return true;
	}
	return false;
}


void CustomTypeInfo::removeMember(const std::string& memberName)
{
	TypeMemberInfo* memberInfo = releaseMember(memberName);
	if (memberInfo != nullptr)
	{
		removedMembers.emplace_back(memberInfo);
	}
}


unsigned char* CustomTypeInfo::getDefaultInstance()
{
	return defaultData;
}


bool CustomTypeInfo::isCustomType() const
{
	return true;
}


jitcat::AST::CatClassDefinition* CustomTypeInfo::getClassDefinition()
{
	return classDefinition;
}


void CustomTypeInfo::placementConstruct(unsigned char* buffer, std::size_t bufferSize) const
{
	auto iter = instances.find(buffer);
	if (iter == instances.end())
	{
		instances.insert(buffer);
	}
	
	if (defaultConstructorFunction != nullptr
		&& (!Configuration::enableLLVM || dylib != nullptr))
	{
		if constexpr (Configuration::enableLLVM)
		{
			reinterpret_cast<void(*)(unsigned char*)>(defaultConstructorFunction->getFunctionAddress(FunctionType::Auto).functionAddress)(buffer);
		}
		else
		{
			std::any base = reinterpret_cast<CustomObject*>(buffer);
			CatRuntimeContext tempContext("temp");
			defaultConstructorFunction->call(&tempContext, base, {});
		}
	}
	else
	{
		createDataCopy(defaultData, typeSize, buffer, bufferSize);
	}
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		if (bufferSize > 0 && buffer != nullptr)
		{
			std::cout << "(CustomTypeInfo::placementConstruct) Placement constructed " << typeName << " at "<< std::hex << reinterpret_cast<uintptr_t>(buffer) << "\n";
		}
	}
}


void CustomTypeInfo::placementDestruct(unsigned char* buffer, std::size_t bufferSize)
{
	ReflectableHandle::nullifyObjectHandles(buffer, this);

	if (destructorFunction != nullptr 
		&& (!Configuration::enableLLVM || dylib != nullptr))
	{
		if constexpr (Configuration::enableLLVM)
		{
			assert(destructorFunction->getFunctionAddress(FunctionType::Auto).functionAddress != 0);
			reinterpret_cast<void(*)(unsigned char*)>(destructorFunction->getFunctionAddress(FunctionType::Auto).functionAddress)(buffer);
		}
		else
		{
			std::any base = reinterpret_cast<CustomObject*>(buffer);
			CatRuntimeContext tempContext("temp");
			destructorFunction->call(&tempContext, base, {});
		}
	}
	else
	{
		instanceDestructorInPlace(buffer);
	}
	removeInstance(buffer);
	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		if (bufferSize > 0 && buffer != nullptr)
		{
			std::cout << "(CustomTypeInfo::placementDestruct) Placement destructed " << typeName << " at " << std::hex << reinterpret_cast<uintptr_t>(buffer) << "\n";
		}
	}
}


void CustomTypeInfo::copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
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


void CustomTypeInfo::moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
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
			unsigned long long memberOffset = static_cast<CustomMemberInfo*>(iter->second)->getOrdinal();
			iter->second->getType().moveConstruct(&targetBuffer[memberOffset], 
												  iter->second->getType().getTypeSize(), 
												  &sourceBuffer[memberOffset], 
												  iter->second->getType().getTypeSize());
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


bool CustomTypeInfo::isTriviallyCopyable() const
{
	return triviallyCopyable;
}


bool Reflection::CustomTypeInfo::isTriviallyConstructable() const
{
	return triviallyConstructable;
}


bool CustomTypeInfo::canBeDeleted() const
{
	return dependentTypes.size() == 0 && instances.size() == 0;
}


llvm::orc::JITDylib* CustomTypeInfo::getDylib() const
{
	return dylib;
}


void CustomTypeInfo::setDylib(llvm::orc::JITDylib* generatedDylib)
{
	dylib = generatedDylib;
}


HandleTrackingMethod CustomTypeInfo::getHandleTrackingMethod() const
{
	return trackingMethod;
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


void CustomTypeInfo::instanceDestructorInPlace(unsigned char* data)
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
			iter->second->getType().placementDestruct(&data[customMember->getOrdinal()], customMember->getType().getTypeSize());
		}
	}
}


std::size_t CustomTypeInfo::addReflectableHandle(unsigned char* defaultValue, TypeInfo* objectType)
{
	unsigned char* data = increaseDataSize(sizeof(ReflectableHandle));
	std::size_t offset = (data - defaultData);
	if (defaultData == nullptr)
	{
		offset = 0;
	}

	std::set<unsigned char*>::iterator end = instances.end();
	for (std::set<unsigned char*>::iterator iter = instances.begin(); iter != end; ++iter)
	{
		new ((unsigned char*)(*iter) + offset) ReflectableHandle(defaultValue, objectType);
	}
	new (data) ReflectableHandle(defaultValue, objectType);
	return offset;
}


unsigned char* CustomTypeInfo::increaseDataSize(std::size_t amount)
{
	std::size_t oldSize = typeSize;
	increaseDataSize(defaultData, amount, typeSize);
	typeSize += amount;
	if (JitCat::get()->getHasPrecompiledExpression())
	{
		std::string typeSizeGlobal = Tools::append("__sizeOf:", getTypeName());
		JitCat::get()->setPrecompiledGlobalVariable(typeSizeGlobal, typeSize);
	}
	std::set<unsigned char*> oldInstances = instances;
	std::set<unsigned char*>::iterator end = oldInstances.end();
	instances.clear();
	for (std::set<unsigned char*>::iterator iter = oldInstances.begin(); iter != end; ++iter)
	{
		unsigned char* data = (unsigned char*)(*iter);
		increaseDataSize(data, amount, oldSize);
		instances.insert(data);
	}
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
		ReflectableHandle::replaceCustomObjects(oldData, this, data, this);
		placementDestruct(oldData, oldSize);
		delete[] oldData;
	}
	else
	{
		data = new unsigned char[newSize];
		if constexpr (Configuration::logJitCatObjectConstructionEvents)
		{
			std::cout << "(CustomTypeInfo::increaseDataSize) Allocated buffer of size " << std::dec << newSize << ": " << std::hex << reinterpret_cast<uintptr_t>(data) << "\n";
		}
		ReflectableHandle::replaceCustomObjects(oldData, this, data, this);
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
			unsigned long long memberOffset = static_cast<CustomMemberInfo*>(iter->second)->getOrdinal();
			iter->second->getType().copyConstruct(&copyData[memberOffset], iter->second->getType().getTypeSize(), 
												  &sourceData[memberOffset], iter->second->getType().getTypeSize());
		}
	}
	else
	{
		memcpy(copyData, sourceData, sourceSize);
	}
	if (trackingMethod == HandleTrackingMethod::InternalHandlePointer && copySize > sizeof(ReflectableHandle*))
	{
		memset(copyData, 0, sizeof(ReflectableHandle*));
	}
}


void CustomTypeInfo::removeInstance(unsigned char* instance)
{
	auto iter = instances.find(instance);
	if (iter != instances.end())
	{
		instances.erase(iter);
	}
}
