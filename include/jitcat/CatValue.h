/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatError.h"
#include "CatGenericType.h"
#include "CatType.h"
#include "MemberReferencePtr.h"

#include <any>


class CatValue
{
public:
	CatValue(): type (CatType::Void), intValue(0), memberReference(nullptr){}
	CatValue(const char* value): stringValue(new std::string(value)), type(CatType::String), memberReference(nullptr) {};
	CatValue(const std::string& value): stringValue(new std::string(value)), type(CatType::String), memberReference(nullptr) {};
	CatValue(float floatValue): floatValue(floatValue), type(CatType::Float), memberReference(nullptr) {};
	CatValue(int intValue): intValue(intValue), type(CatType::Int), memberReference(nullptr) {};
	CatValue(bool boolValue): boolValue(boolValue), type(CatType::Bool), memberReference(nullptr) {};
	CatValue(const CatError& errorValue): errorValue(new CatError(errorValue.message)), type(CatType::Error), memberReference(nullptr) {};
	CatValue(const MemberReferencePtr& memberReference): intValue(0), memberReference(memberReference), type(CatType::Object) {};
	CatValue(const CatValue& other);
	~CatValue();
	CatType getValueType() const;
	float getFloatValue() const;
	int getIntValue() const;
	bool getBoolValue() const;
	const std::string& getStringValue() const;
	const CatError& getErrorValue() const;
	MemberReferencePtr getCustomTypeValue() const {return memberReference;}
	bool isReference() const;
	//These always return a value even if  this CatValue is a CatType::Error or a CatType::Object
	int toIntValue() const;
	float toFloatValue() const;
	bool toBoolValue() const;
	std::string toStringValue() const;

	CatGenericType getGenericType() const;

	CatValue& operator= (const CatValue& other);

	void printValue() const;

	std::any toAny();

private:
	union
	{
		float floatValue;
		int intValue;
		bool boolValue;
		std::string* stringValue;
		CatError* errorValue;
	};
	MemberReferencePtr memberReference;
	CatType type;
};
