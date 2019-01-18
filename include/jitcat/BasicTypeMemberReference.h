/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

struct TypeMemberInfo;

#include "MemberReference.h"
#include "ReflectableHandle.h"


#include <string>


//See MemberReference.h
//Stores a reference to a basic type member of an object.
//The class is specialized for the four supported basic types.
//A handle to the parent object is stored to check if the parent object still exists
template <typename T>
class BasicTypeMemberReference: public MemberReference
{
public:
	BasicTypeMemberReference(T& basicMember, TypeMemberInfo* memberInfo, Reflectable* parentObject, bool isWritable): MemberReference(memberInfo) {};
};


template <>
class BasicTypeMemberReference<float>: public MemberReference
{
public:
	BasicTypeMemberReference(float& member, TypeMemberInfo* memberInfo, Reflectable* parentObject, bool isWritable): MemberReference(memberInfo), member(member), isWritable(isWritable), parentObject(parentObject) {};

	inline virtual float getFloat() const override final
	{
		if (parentObject.get() != nullptr)
		{
			return member;
		}
		else
		{
			return 0.0f;
		}
	}
	inline virtual std::any getAny() const override final {return getFloat();}
	inline virtual void setFloat(float value) override final;
	inline virtual Reflectable* getParentObject() const override final { return parentObject.get(); }
	inline virtual SpecificMemberType getType() const override final { return SpecificMemberType::CatType; }
	inline virtual CatType getCatType() const override final { return CatType::Float; }
	inline virtual const std::string getValueAsString() const override final
	{
		if (parentObject.getIsValid()) { return Tools::makeString(member); }
		else { return "invalid"; }
	}
private:
	ReflectableHandle parentObject;
	float& member;
	bool isWritable;
};


template <>
class BasicTypeMemberReference<int>: public MemberReference
{
public:
	BasicTypeMemberReference(int& member, TypeMemberInfo* memberInfo, Reflectable* parentObject, bool isWritable): MemberReference(memberInfo), member(member), isWritable(isWritable), parentObject(parentObject) {};
	inline virtual int getInt() const override final
	{
		if (parentObject.get() != nullptr)
		{
			return member;
		}
		else
		{
			return 0;
		}
	}
	inline virtual std::any getAny() const override final {return getInt();}
	inline virtual void setInt(int value) override final;
	inline virtual Reflectable* getParentObject() const override final { return parentObject.get(); }
	inline virtual SpecificMemberType getType() const override final { return SpecificMemberType::CatType; }
	inline virtual CatType getCatType() const override final { return CatType::Int; }
	inline virtual const std::string getValueAsString() const override final
	{
		if (parentObject.getIsValid()) { return Tools::makeString(member); }
		else { return "invalid"; }
	}
private:
	ReflectableHandle parentObject;
	int& member;
	bool isWritable;
};


template <>
class BasicTypeMemberReference<bool>: public MemberReference
{
public:
	BasicTypeMemberReference(bool& member, TypeMemberInfo* memberInfo, Reflectable* parentObject, bool isWritable): MemberReference(memberInfo), member(member), isWritable(isWritable), parentObject(parentObject) {};
	inline virtual bool getBool() const override final
	{
		if (parentObject.get() != nullptr)
		{
			return member;
		}
		else
		{
			return false;
		}
	}
	inline virtual std::any getAny() const override final {return getBool();}
	inline virtual void setBool(bool value) override final;
	inline virtual Reflectable* getParentObject() const override final { return parentObject.get(); }
	inline virtual SpecificMemberType getType() const override final { return SpecificMemberType::CatType; }
	inline virtual CatType getCatType() const override final { return CatType::Bool; }
	inline virtual const std::string getValueAsString() const override final
	{
		if (parentObject.getIsValid()) { return Tools::makeString(member); }
		else { return "invalid"; }
	}
private:
	ReflectableHandle parentObject;
	bool& member;
	bool isWritable;
};


template <>
class BasicTypeMemberReference<std::string>: public MemberReference
{
public:
	BasicTypeMemberReference(std::string& member, TypeMemberInfo* memberInfo, Reflectable* parentObject, bool isWritable): MemberReference(memberInfo), member(member), isWritable(isWritable), parentObject(parentObject) {};
	inline virtual const std::string& getString() const override final
	{
		if (parentObject.get() != nullptr)
		{
			return member;
		}
		else
		{
			return Tools::empty;
		}
	}
	inline virtual std::any getAny() const override final {return getString();}
	inline virtual void setString(const std::string& value) override final;
	inline virtual Reflectable* getParentObject() const override final { return parentObject.get(); }
	inline virtual SpecificMemberType getType() const override final { return SpecificMemberType::CatType; }
	inline virtual CatType getCatType() const override final { return CatType::String; }
	inline virtual const std::string getValueAsString() const override final
	{
		if (parentObject.getIsValid()) { return member; }
		else { return "invalid"; }
	}
private:
	ReflectableHandle parentObject;
	std::string& member;
	bool isWritable;
};


#include "BasicTypeMemberReferenceHeaderImplementation.h"