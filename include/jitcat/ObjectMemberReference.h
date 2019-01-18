/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "MemberReference.h"
#include "MemberReferencePtr.h"
#include "ReflectableHandle.h"


//See MemberReference.h
//Stores a class/struct type reference to a member of an object.
template<typename T>
class ObjectMemberReference: public MemberReference
{
public:
	ObjectMemberReference(T* object, TypeMemberInfo* memberInfo, TypeInfo* typeInfo): MemberReference(memberInfo), object(object), typeInfo(typeInfo){}
	inline virtual MemberReferencePtr getMemberReference(const std::string& memberOrIndex) override final;
	T* getObject() const { return static_cast<T*>(object.get()); }
	inline virtual std::any getAny() const override final { return getObject();}
	inline virtual SpecificMemberType getType() const override final{ return SpecificMemberType::NestedType; }
	inline virtual const char* getCustomTypeName() const override final;
	inline virtual CatType getCatType() const override final { return CatType::Object; }
	inline virtual Reflectable* getParentObject() const override final{ return object.get(); }
	inline virtual const std::string getValueAsString() const override final
	{
		if (object.getIsValid()) { return Tools::makeString(object.get()); }
		else { return "invalid"; }
	}

private:
	ReflectableHandle object;
	TypeInfo* typeInfo;
};


#include "ObjectMemberReferenceHeaderImplementation.h"