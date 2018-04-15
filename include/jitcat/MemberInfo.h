/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class MemberReference;
class TypeInfo;
#include "CatGenericType.h"
#include "CatType.h"
#include "ContainerType.h"
#include "MemberTypeFlags.h"
#include "MemberReferencePtr.h"
#include "SpecificMemberType.h"

#include <memory>
#include <string>


//This struct contains type information on a single member of a reflectable object
//A member can be:
//- A basic type (int, float, bool, std::string)
//- A container type, vector<T*> or map<std::string, T*>, where T is a reflectable type
//- A nested reflectable type pointer

//See TypeInfo.h for more information
struct TypeMemberInfo
{
	TypeMemberInfo(): specificType(SpecificMemberType::None), catType(CatType::Unknown), nestedType(nullptr), containerType(ContainerType::None), isConst(false), isWritable(false) {}
	TypeMemberInfo(const std::string& memberName, CatType type, bool isConst, bool isWritable): memberName(memberName), specificType(SpecificMemberType::CatType), catType(type), nestedType(nullptr), containerType(ContainerType::None), isConst(isConst), isWritable(isWritable) {}
	TypeMemberInfo(const std::string& memberName, TypeInfo* type, bool isConst, bool isWritable): memberName(memberName), specificType(SpecificMemberType::NestedType), catType(CatType::Object), nestedType(type), containerType(ContainerType::None), isConst(isConst), isWritable(isWritable) {}
	TypeMemberInfo(const std::string& memberName, ContainerType type, TypeInfo* itemType, bool isConst, bool isWritable): memberName(memberName), specificType(SpecificMemberType::ContainerType), catType(CatType::Object), nestedType(itemType), containerType(type), isConst(isConst), isWritable(isWritable) {}
	virtual ~TypeMemberInfo() {};
	inline virtual MemberReferencePtr getMemberReference(MemberReferencePtr& base) { return nullptr; }

	CatGenericType toGenericType() const;

	std::string getFullTypeName() const;

	SpecificMemberType specificType;
	CatType catType;
	TypeInfo* nestedType;

	//When the member is a container, catType or nestedType will be set to the item type of the container
	ContainerType containerType;
	bool isConst;
	bool isWritable;

	std::string memberName;
};


//Implements a TypeMemberInfo for container types.
template<typename T, typename U>
struct ContainerMemberInfo: public TypeMemberInfo
{
	ContainerMemberInfo(const std::string& memberName, U T::* memberPointer, ContainerType type, TypeInfo* itemType, bool isConst): TypeMemberInfo(memberName, type, itemType, isConst, false), memberPointer(memberPointer) {}

	inline virtual MemberReferencePtr getMemberReference(MemberReferencePtr& base);
	U T::* memberPointer;
};


//Implements a TypeMemberInfo for class/struct pointer types that are reflectable.
template<typename T, typename U>
struct ClassPointerMemberInfo: public TypeMemberInfo
{
	ClassPointerMemberInfo(const std::string& memberName, U* T::* memberPointer, TypeInfo* type, bool isConst, bool isWritable): TypeMemberInfo(memberName, type, isConst, isWritable), memberPointer(memberPointer) {}

	inline virtual MemberReferencePtr getMemberReference(MemberReferencePtr& base);
	U* T::* memberPointer;
};

//Implements a TypeMemberInfo for class/struct types that are reflectable.
template<typename T, typename U>
struct ClassObjectMemberInfo: public TypeMemberInfo
{
	ClassObjectMemberInfo(const std::string& memberName, U T::* memberPointer, TypeInfo* type, bool isConst, bool isWritable): TypeMemberInfo(memberName, type, isConst, isWritable), memberPointer(memberPointer) {}

	inline virtual MemberReferencePtr getMemberReference(MemberReferencePtr& base);
	U T::* memberPointer;
};


//Implements a TypeMemberInfo for a unique_ptr to class/struct types that are reflectable.
template<typename T, typename U>
struct ClassUniquePtrMemberInfo: public TypeMemberInfo
{
	ClassUniquePtrMemberInfo(const std::string& memberName, std::unique_ptr<U> T::* memberPointer, TypeInfo* type, bool isConst, bool isWritable): TypeMemberInfo(memberName, type, isConst, isWritable), memberPointer(memberPointer) {}

	inline virtual MemberReferencePtr getMemberReference(MemberReferencePtr& base);
	std::unique_ptr<U> T::* memberPointer;
};



//Implements a TypeMemberInfo for basic types.
template<typename T, typename U>
struct BasicTypeMemberInfo: public TypeMemberInfo
{
	BasicTypeMemberInfo(const std::string& memberName, U T::* memberPointer, CatType type, bool isConst, bool isWritable): TypeMemberInfo(memberName, type, isConst, isWritable), memberPointer(memberPointer) {}
	inline virtual MemberReferencePtr getMemberReference(MemberReferencePtr& base);
	U T::* memberPointer;
};


#include "MemberInfoHeaderImplementation.h"
