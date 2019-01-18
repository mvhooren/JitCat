/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class TypeInfo;
#include "CatInfixOperatorType.h"
#include "CatType.h"
#include "ContainerType.h"


#include <any>
#include <string>


class CatGenericType
{
public:
	CatGenericType();
	CatGenericType(CatType catType);
	CatGenericType(TypeInfo* objectType);
	CatGenericType(ContainerType containerType, TypeInfo* itemType);
	CatGenericType(const std::string& message);

	bool operator== (const CatType other) const;
	bool operator== (const CatGenericType& other) const;
	bool operator!= (const CatType other) const;
	bool operator!= (const CatGenericType& other) const;

	bool isUnknown() const;
	bool isValidType() const;
	bool isBasicType() const;
	bool isBoolType() const;
	bool isIntType() const;
	bool isFloatType() const;
	bool isStringType() const;
	bool isScalarType() const;
	bool isVoidType() const;
	bool isObjectType() const;
	bool isContainerType() const;
	bool isVectorType() const;
	bool isMapType() const;
	bool isEqualToBasicCatType(CatType catType) const;
	CatType getCatType() const;

	CatGenericType getContainerItemType() const;
	const char* getObjectTypeName() const;

	const std::string& getErrorMessage() const;

	CatGenericType getInfixOperatorResultType(CatInfixOperatorType oper, const CatGenericType& rightType);

	std::string toString() const;

	TypeInfo* getObjectType() const;

	//This will cast the pointer to the C++ type associated with this CatGenericType and returns it as a std::any
	std::any createAnyOfType(void* pointer);

private:
	enum class SpecificType
	{
		None,
		Error,
		CatType,
		ObjectType,
		ContainerType
	};

	SpecificType specificType;
	CatType catType;
	//not owned
	TypeInfo* nestedType;

	//When the member is a container, catType or nestedType will be set to the item type of the container
	ContainerType containerType;

	std::string error;
};
