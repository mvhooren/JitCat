/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "MemberReference.h"
#include "ReflectableHandle.h"
#include "Tools.h"

//See MemberReference.h
//Stores a reference to a container type member of an object.
//The class is specialized for the two supported container types.
template <typename T>
class ContainerMemberReference: public MemberReference
{
public:
	ContainerMemberReference(T& container, TypeMemberInfo* memberInfo, Reflectable* baseObject, TypeInfo* itemTypeInfo): MemberReference(memberInfo) {};
	virtual MemberReferencePtr getMemberReference(const std::string& index) { return nullptr; }
};


template <typename U>
class ContainerMemberReference<std::vector<U> >: public MemberReference
{
public:
	ContainerMemberReference(std::vector<U>& container, TypeMemberInfo* memberInfo, Reflectable* parentObject, TypeInfo* itemTypeInfo): MemberReference(memberInfo), container(container), parentObject(parentObject), itemTypeInfo(itemTypeInfo) {};

	inline virtual SpecificMemberType getType() const { return SpecificMemberType::ContainerType; }
	inline virtual const char* getCustomTypeName() const;
	inline virtual CatType getCatType() const { return CatType::Object; }
	inline virtual ContainerType getContainerType() const { return ContainerType::Vector; }
	inline virtual std::size_t getContainerSize() const
	{
		if (parentObject.getIsValid())
		{
			return container.size();
		}
		else
		{
			return 0;
		}
	}

	virtual MemberReferencePtr getMemberReference(const std::string& index);

	inline virtual MemberReferencePtr getArrayItemReference(int index);

	inline virtual Reflectable* getParentObject() const { return parentObject.get(); }
	
	inline virtual const std::string getValueAsString() const;

	//Handle to parent object is stored so we can check if it still exists (vectors cant be Reflectable).
	ReflectableHandle parentObject;
	std::vector<U>& container;
	TypeInfo* itemTypeInfo;
};


template <typename U>
class ContainerMemberReference<std::map<std::string, U> >: public MemberReference
{
public:
	ContainerMemberReference(std::map<std::string, U>& container, TypeMemberInfo* memberInfo, Reflectable* parentObject, TypeInfo* itemTypeInfo): MemberReference(memberInfo), container(container), parentObject(parentObject), itemTypeInfo(itemTypeInfo) {};

	inline virtual SpecificMemberType getType() const { return SpecificMemberType::ContainerType; }
	inline virtual const char* getCustomTypeName() const;
	inline virtual CatType getCatType() const { return CatType::Object; }
	inline virtual ContainerType getContainerType() const { return ContainerType::StringMap; }
	inline virtual std::size_t getContainerSize() const
	{
		if (parentObject.getIsValid())
		{
			return container.size();
		}
		else
		{
			return 0;
		}
	}


	inline virtual std::string getMapIndexName(unsigned int index) const
	{
		if (parentObject.getIsValid())
		{
			typename std::map<std::string, U>::const_iterator endIter = container.end();
			unsigned int i = 0;
			for (typename std::map<std::string, U>::const_iterator iter = container.begin(); iter != endIter; ++iter)
			{
				if (i == index)
				{
					return iter->first;
				}
				i++;
			}
		}
		return "";
	}


	inline virtual MemberReferencePtr getMemberReference(const std::string& index);


	inline virtual MemberReferencePtr getArrayItemReference(int index);


	inline virtual Reflectable* getParentObject() const { return parentObject.get(); }

	inline virtual const std::string getValueAsString() const;

	ReflectableHandle parentObject;
	std::map<std::string, U>& container;
	TypeInfo* itemTypeInfo;
};


#include "ContainerMemberReferenceHeaderImplementation.h"