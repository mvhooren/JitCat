/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class Reflectable;
class ReflectableHandle;
class TypeInfo;
struct TypeMemberInfo;
#include "CatType.h"
#include "ContainerType.h"
#include "MemberReferencePtr.h"
#include "MemberTypeFlags.h"
#include "SpecificMemberType.h"
#include "Tools.h"

#include <string>
#include <map>

//When getting a reference to member of an object by string using a TypeInfo object (see TypeInfo.h), a MemberReference is returned.
//MemberReference stores a direct reference to the member.
//Depending on the type of the member, a different child class of MemberReference is created.
//This can be one of:
// - ObjectMemberReference, a reference to a class/struct member.
// - BasicTypeMemberReference, a reference to a basic type (int, float, bool or std::string). There is a template specialization for each type.
// - ContainerMemberReference, a reference to a container (std::vector<U> or std::map<std::string, CaseInsensitiveCompare, U>) with a template specialization for each supported container.

//The members/items of an ObjectMemberReferences or ContainerMemberReferences can be accessed using the getMemberReference function.

//To start getting member references of an object by string, create an ObjectMemberReference, passing a pointer to the object and its TypeInfo object.
//Then call getMemberReference on it.
//An easier way is to use a DotNotatedMemberReference object (see DotNotatedMemberReference.h)

//BasicTypeMemberReferences and ContainerMemberReference also store a handle to the base object that the member belongs to so it can check if the base object still exists (it must be Reflectable)

class MemberReference
{
protected:
	virtual ~MemberReference() {};
	friend class MemberReferencePtr;

public:
	MemberReference(TypeMemberInfo* memberInfo): numReferences(0), memberInfo(memberInfo) {};
	inline virtual MemberReferencePtr getMemberReference(const std::string& memberOrIndex) { return nullptr; };
	inline virtual MemberReferencePtr getArrayItemReference(int index) { return nullptr; };
	inline virtual Reflectable* getParentObject() const { return nullptr; };
	inline virtual float getFloat() const { return 0.0f; }
	inline virtual int getInt() const { return 0; }
	inline virtual bool getBool() const { return false; }
	inline virtual const std::string& getString() const { return Tools::empty; }

	inline virtual void setFloat(float value) {}
	inline virtual void setInt(int value) {}
	inline virtual void setBool(bool value) {}
	inline virtual void setString(const std::string& value) {}

	inline virtual const std::string getValueAsString() const { return Tools::empty; }

	inline virtual SpecificMemberType getType() const { return SpecificMemberType::None; }
	inline virtual CatType getCatType() const { return CatType::Error; }
	inline virtual const char* getCustomTypeName() const { return ""; }
	inline virtual ContainerType getContainerType() const { return ContainerType::None; }
	inline virtual std::size_t getContainerSize() const { return 0; }
	inline virtual std::string getMapIndexName(unsigned int index) const { return ""; }

	CatGenericType getGenericType() const;

	void incrementReferenceCounter();
	bool decrementReferenceCounter();

	//Can be nullptr
	TypeMemberInfo* getMemberInfo() const;

protected:
	TypeMemberInfo* memberInfo;

private:
	int numReferences;

};