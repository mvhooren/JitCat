/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatGenericType.h"
#include "TypeInfo.h"


CatGenericType::CatGenericType():
	specificType(SpecificType::None),
	catType(CatType::Unknown),
	nestedType(nullptr),
	containerType(ContainerType::None)
{
}


CatGenericType::CatGenericType(CatType catType):
	specificType(SpecificType::CatType),
	catType(catType),
	nestedType(nullptr),
	containerType(ContainerType::None)
{
}


CatGenericType::CatGenericType(TypeInfo* objectType):
	specificType(SpecificType::ObjectType),
	catType(CatType::Object),
	nestedType(objectType),
	containerType(ContainerType::None)
{
}


CatGenericType::CatGenericType(ContainerType containerType, TypeInfo* itemType):
	specificType(SpecificType::ContainerType),
	catType(CatType::Object),
	nestedType(itemType),
	containerType(containerType)
{
}


CatGenericType::CatGenericType(const std::string& message):
	specificType(SpecificType::Error),
	catType(CatType::Error),
	nestedType(nullptr),
	containerType(ContainerType::None),
	error(message)
{
}


bool CatGenericType::operator==(const CatType other) const
{
	return catType == other;
}


bool CatGenericType::operator==(const CatGenericType& other) const
{
	if (specificType == other.specificType)
	{
		switch (specificType)
		{
			default:
			case SpecificType::None:				return true;
			case SpecificType::Error:				return error == other.error;
			case SpecificType::CatType:			return catType == other.catType;
			case SpecificType::ObjectType:		return nestedType->getTypeName() == other.nestedType->getTypeName();
			case SpecificType::ContainerType:		return containerType == other.containerType && nestedType->getTypeName() == other.nestedType->getTypeName();
		}
	}
	else
	{
		return false;
	}
}


bool CatGenericType::isValidType() const
{
	return specificType != SpecificType::Error 
		   && (specificType != SpecificType::CatType || catType != CatType::Error)
		   && (specificType != SpecificType::ObjectType || nestedType != nullptr);
}


bool CatGenericType::isBasicType() const
{
	return specificType == SpecificType::CatType && ::isBasicType(catType);
}


bool CatGenericType::isBoolType() const
{
	return specificType == SpecificType::CatType && catType == CatType::Bool;
}


bool CatGenericType::isIntType() const
{
	return specificType == SpecificType::CatType && catType == CatType::Int;
}


bool CatGenericType::isFloatType() const
{
	return specificType == SpecificType::CatType && catType == CatType::Float;
}


bool CatGenericType::isStringType() const
{
	return specificType == SpecificType::CatType && catType == CatType::String;
}


bool CatGenericType::isScalarType() const
{
	return specificType == SpecificType::CatType && isScalar(catType);
}


bool CatGenericType::isVoidType() const
{
	return specificType == SpecificType::CatType && catType == CatType::Void;
}


bool CatGenericType::isObjectType() const
{
	return specificType == SpecificType::ObjectType;
}


bool CatGenericType::isContainerType() const
{
	return specificType == SpecificType::ContainerType;
}


bool CatGenericType::isVectorType() const
{
	return specificType == SpecificType::ContainerType && containerType == ContainerType::Vector;
}


bool CatGenericType::isMapType() const
{
	return specificType == SpecificType::ContainerType && containerType == ContainerType::StringMap;
}


bool CatGenericType::isEqualToBasicCatType(CatType catType_) const
{
	return isBasicType() && catType == catType_;
}


CatType CatGenericType::getCatType() const
{
	return catType;
}


CatGenericType CatGenericType::getContainerItemType() const
{
	if (specificType == SpecificType::ContainerType
		&& nestedType != nullptr)
	{
		return CatGenericType(nestedType);
	}
	else
	{
		return CatGenericType("Not a container.");
	}
}


const char* CatGenericType::getObjectTypeName() const
{
	if (specificType == SpecificType::ObjectType
		&& nestedType != nullptr)
	{
		return nestedType->getTypeName();
	}
	else
	{
		return nullptr;
	}
}


const std::string& CatGenericType::getErrorMessage() const
{
	return error;
}


CatGenericType CatGenericType::getInfixOperatorResultType(CatInfixOperatorType oper, const CatGenericType& rightType)
{
	if (!rightType.isValidType())
	{
		return rightType;
	}
	else if (!isValidType())
	{
		return *this;
	}
	else if (isBasicType()
			 && rightType.isBasicType())
	{
		CatType rCatType = rightType.catType;
		switch (oper)
		{
			default:
			case CatInfixOperatorType::Assign:			break;
			case CatInfixOperatorType::Plus:			
				if (catType == CatType::String || rCatType == CatType::String)
				{
					return CatGenericType(CatType::String);
				}
				//Intentional lack of break
			case CatInfixOperatorType::Minus:
			case CatInfixOperatorType::Multiply:
			case CatInfixOperatorType::Divide:
				if (isScalar(catType) && isScalar(rCatType))
				{
					return CatGenericType((catType == CatType::Int && rCatType == CatType::Int) ? CatType::Int : CatType::Float);
				}
				break;
			case CatInfixOperatorType::Modulo:
				if (isScalar(catType) && isScalar(rCatType))
				{
					return CatGenericType(CatType::Int);
				}
				break;
			case CatInfixOperatorType::Greater:
			case CatInfixOperatorType::Smaller:
			case CatInfixOperatorType::GreaterOrEqual:
			case CatInfixOperatorType::SmallerOrEqual:
				if (isScalar(catType) && isScalar(rCatType))
				{
					return CatGenericType(CatType::Bool);
				}
				break;
			case CatInfixOperatorType::Equals:
			case CatInfixOperatorType::NotEquals:
				if (catType == rCatType
					|| (isScalar(catType) && isScalar(rCatType)))
				{
					return CatGenericType(CatType::Bool);
				}
				break;
			case CatInfixOperatorType::LogicalAnd:
			case CatInfixOperatorType::LogicalOr:
				if (catType == CatType::Bool && rCatType == CatType::Bool)
				{
					return CatGenericType(CatType::Bool);
				}
				break;
		}
	}

	return CatGenericType(Tools::append("Invalid operation: ", toString(), " ", ::toString(oper), " ", rightType.toString()));
}


std::string CatGenericType::toString() const
{
	switch (specificType)
	{
		default:					return "Unknown";
		case SpecificType::Error:				return "Error";
		case SpecificType::CatType:			return ::toString(catType);
		case SpecificType::ObjectType:		return nestedType->getTypeName();
		case SpecificType::ContainerType:
			if (containerType == ContainerType::Vector)
			{
				return  std::string("list of ") + nestedType->getTypeName();
			}
			else if (containerType == ContainerType::StringMap)
			{
				return std::string("map of ") + nestedType->getTypeName();
			}
			else
			{
				return "Unknown";
			}
	}
}


TypeInfo* CatGenericType::getObjectType() const
{
	return nestedType;
}

