/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CustomTypeInfo.h"
#include "CustomTypeMemberInfo.h"
#include "MemberReference.h"
#include "Tools.h"
#include "TypeRegistry.h"


CustomTypeInfo::CustomTypeInfo(const char* typeName, bool isConstType):
	TypeInfo(typeName),
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
		CustomTypeInstance* instance = new CustomTypeInstance(createDataCopy(source->data), this);
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
	return createDataCopy(defaultData);
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
	std::map<std::string, TypeMemberInfo*>::iterator end = members.end();
	for (std::map<std::string, TypeMemberInfo*>::iterator iter = members.begin(); iter != end; ++iter)
	{
		if (iter->second->catType == CatType::String)
		{
			std::string* string;
			unsigned int offset = static_cast<CustomBasicTypeMemberInfo<std::string>*>(iter->second)->memberOffset;
			memcpy(&string, &data[offset], sizeof(std::string*));
			delete string;
		}
		else if (iter->second->catType == CatType::Object)
		{
			MemberReferencePtr* reference;
			unsigned int offset = static_cast<CustomBasicTypeMemberInfo<std::string>*>(iter->second)->memberOffset;
			memcpy(&reference, &data[offset], sizeof(MemberReferencePtr*));
			delete reference;
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

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<float>(memberName, offset, CatType::Float, isConst, isWritable);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	members[lowerCaseMemberName] = memberInfo;
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

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<int>(memberName, offset, CatType::Int, isConst, isWritable);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	members[lowerCaseMemberName] = memberInfo;
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

	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<bool>(memberName, offset, CatType::Bool, isConst, isWritable);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	members[lowerCaseMemberName] = memberInfo;
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
	TypeMemberInfo* memberInfo = new CustomBasicTypeMemberInfo<std::string>(memberName, offset, CatType::String, isConst, isWritable);
	std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
	members[lowerCaseMemberName] = memberInfo;
	return memberInfo;
}


TypeMemberInfo* CustomTypeInfo::addObjectMember(const std::string& memberName, const std::string& memberTypeName, MemberReference* defaultValue, bool isWritable, bool isConst)
{
	if (defaultValue != nullptr
		&& Tools::toLowerCase(defaultValue->getCustomTypeName()) == Tools::toLowerCase(memberTypeName))
	{
		isTriviallyCopyable = false;
		MemberReferencePtr* memberReference = new MemberReferencePtr(defaultValue);
		memberReference->setOriginalReference(memberReference);
		unsigned char* data = increaseDataSize(sizeof(MemberReferencePtr*));
		unsigned int offset = (unsigned int)(data - defaultData);
		if (defaultData == nullptr)
		{
			offset = 0;
		}

		std::set<CustomTypeInstance*>::iterator end = instances.end();
		for (std::set<CustomTypeInstance*>::iterator iter = instances.begin(); iter != end; ++iter)
		{
			MemberReferencePtr* newReference = new MemberReferencePtr(defaultValue);
			newReference->setOriginalReference(newReference);
			memcpy((*iter)->data + offset, &newReference, sizeof(MemberReferencePtr*));
		}

		memcpy(data, &memberReference, sizeof(MemberReferencePtr*));
		TypeMemberInfo* memberInfo = new CustomTypeObjectMemberInfo(memberName, offset, TypeRegistry::get()->getOrCreateTypeInfo(defaultValue->getCustomTypeName()), isConst);
		std::string lowerCaseMemberName = Tools::toLowerCase(memberName);
		members[lowerCaseMemberName] = memberInfo;
		return memberInfo;
	}
	return nullptr;
}


void CustomTypeInfo::removeMember(const std::string& memberName)
{
	std::map<std::string, TypeMemberInfo*>::iterator iter = members.find(Tools::toLowerCase(memberName));
	if (iter != members.end())
	{
		//TypeMemberInfo is leaked here, but this is an easy way to prevent previous versions from crashing so it is acceptable.
		//Removing members is rare anyway.
		//Maybe make TypeMemberInfos reference counted in the future.
		members.erase(iter);
	}
}


void CustomTypeInfo::renameMember(const std::string& oldMemberName, const std::string& newMemberName)
{
	std::map<std::string, TypeMemberInfo*>::iterator iter = members.find(Tools::toLowerCase(oldMemberName));
	if (iter != members.end() && members.find(Tools::toLowerCase(newMemberName)) == members.end())
	{
		TypeMemberInfo* memberInfo = iter->second;
		memberInfo->memberName = newMemberName;
		members.erase(iter);
		std::string lowerCaseMemberName = Tools::toLowerCase(newMemberName);
		members[lowerCaseMemberName] = memberInfo;
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
	data = new unsigned char[newSize];
	if (oldData != nullptr
		&& oldSize != 0)
	{
		memcpy(data, oldData, oldSize);
	}
	//Initialise memory to zero
	memset(data + oldSize, 0, amount);
	delete[] oldData;

}


unsigned char* CustomTypeInfo::createDataCopy(unsigned char* otherData)
{
	//Create copies of strings and member references
	if (otherData != nullptr)
	{
		unsigned char* instanceData = new unsigned char[typeSize];
		memcpy(instanceData, otherData, typeSize);
		if (!isTriviallyCopyable)
		{
			std::map<std::string, TypeMemberInfo*>::iterator end = members.end();
			for (std::map<std::string, TypeMemberInfo*>::iterator iter = members.begin(); iter != end; ++iter)
			{
				if (iter->second->catType == CatType::String)
				{
					std::string* originalString;
					unsigned int offset = static_cast<CustomBasicTypeMemberInfo<std::string>*>(iter->second)->memberOffset;
					memcpy(&originalString, &otherData[offset], sizeof(std::string*));
					std::string* stringCopy = new std::string(*originalString);
					memcpy(instanceData + offset, &stringCopy, sizeof(std::string*));
				}
				else if (iter->second->catType == CatType::Object)
				{
					MemberReferencePtr* originalReference;
					unsigned int offset = static_cast<CustomTypeObjectMemberInfo*>(iter->second)->memberOffset;
					memcpy(&originalReference, &otherData[offset], sizeof(MemberReferencePtr*));
					MemberReferencePtr* copy = new MemberReferencePtr(originalReference->getPointer());
					copy->setOriginalReference(copy);
					memcpy(instanceData + offset, &copy, sizeof(MemberReferencePtr*));
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
