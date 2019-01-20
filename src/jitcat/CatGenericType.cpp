/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatGenericType.h"
#include "CatLog.h"
#include "Tools.h"
#include "TypeInfo.h"

#include <cassert>


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
	catType(CatType::Unknown),
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


bool CatGenericType::operator!=(const CatType other) const
{
	return !this->operator==(other);
}


bool CatGenericType::operator!=(const CatGenericType& other) const
{
	return !this->operator==(other);
}


bool CatGenericType::isUnknown() const
{
	return catType == CatType::Unknown;
}


bool CatGenericType::isValidType() const
{
	return specificType != SpecificType::Error 
		   && specificType != SpecificType::None 
		   && (specificType != SpecificType::CatType || catType != CatType::Unknown)
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
					return CatGenericType(catType);
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


std::any CatGenericType::createAnyOfType(uintptr_t pointer)
{
	switch (specificType)
	{
		case SpecificType::CatType:
		{
			switch (catType)
			{
				case CatType::Int:		return std::any(reinterpret_cast<int*>(pointer));
				case CatType::Float:	return std::any(reinterpret_cast<float*>(pointer));
				case CatType::Bool:		return std::any(reinterpret_cast<bool*>(pointer));
				case CatType::String:	return std::any(reinterpret_cast<std::string*>(pointer));
			}
		} break;
		case SpecificType::ObjectType:
		{
			return nestedType->getTypeCaster()->cast(pointer);
		} break;
		case SpecificType::ContainerType:
		{
			if (containerType == ContainerType::Vector)
			{
				return nestedType->getTypeCaster()->castToVectorOf(pointer);
			}
			else if (containerType == ContainerType::StringMap)
			{
				return nestedType->getTypeCaster()->castToStringIndexedMapOf(pointer);
			}
		} break;
	}
	return std::any();
}


std::any CatGenericType::createDefault() const
{
	switch (specificType)
	{
		case SpecificType::CatType:
		{
			switch (catType)
			{
				case CatType::Int:		return 0;
				case CatType::Float:	return 0.0f;
				case CatType::Bool:		return false;
				case CatType::String:	return std::string();
			}
		} break;
		case SpecificType::ObjectType:
		{
			return nestedType->getTypeCaster()->getNull();
		} break;
		case SpecificType::ContainerType:
		{
			if (containerType == ContainerType::Vector)
			{
				return nestedType->getTypeCaster()->getNullVector();
			}
			else if (containerType == ContainerType::StringMap)
			{
				return nestedType->getTypeCaster()->getNullStringIndexedMap();
			}
		} break;
	}
	return std::any();
}


std::any CatGenericType::convertToType(std::any value, const CatGenericType& valueType) const
{
	if (this->operator==(valueType))
	{
		return value;
	}
	else if (isBasicType() && valueType.isBasicType())
	{
		switch(catType)
		{
			case CatType::Int:
			{
				switch (valueType.getCatType())
				{
					case CatType::Float:	return (int)std::any_cast<float>(value);
					case CatType::Bool:		return std::any_cast<bool>(value) ? 1 : 0;
					case CatType::String:	return atoi(std::any_cast<std::string>(value).c_str());
				}
			} break;
			case CatType::Float:
			{
				switch (valueType.getCatType())
				{
					case CatType::Int:		return (float)std::any_cast<int>(value);
					case CatType::Bool:		return std::any_cast<bool>(value) ? 1.0f : 0.0f;
					case CatType::String:	return (float)atof(std::any_cast<std::string>(value).c_str());
				}
			} break;
			case CatType::Bool:
			{
				switch (valueType.getCatType())
				{
					case CatType::Float:	return std::any_cast<float>(value) > 0.0f;
					case CatType::Int:		return std::any_cast<int>(value) > 0;
					case CatType::String:
					{
						std::string strValue = std::any_cast<std::string>(value);
						return  strValue == "true" || atoi(strValue.c_str()) > 0;
					}
				}
			} break;
			case CatType::String:
			{
				switch (valueType.getCatType())
				{
					case CatType::Float:
					{
						std::ostringstream ss;
						ss << std::any_cast<float>(value);
						return std::string(ss.str());
					}
					case CatType::Int:
					{
						std::ostringstream ss;
						ss << std::any_cast<int>(value);
						return std::string(ss.str());
					}
					case CatType::Bool:		
					{
						return std::any_cast<bool>(value) ? std::string("1") : std::string("0");
					}
				}
			} break;
		}
	}
	assert(false);
	return createDefault();
}


void CatGenericType::printValue(std::any& value)
{
	switch (specificType)
	{
		case SpecificType::CatType:
		{
			switch (catType)
			{
				case CatType::Int:		CatLog::log(std::any_cast<int>(value)); break;
				case CatType::Float:	CatLog::log(std::any_cast<float>(value)); break;
				case CatType::Bool:		CatLog::log(std::any_cast<bool>(value) ? "true" : "false"); break;
				case CatType::String:	CatLog::log("\""); CatLog::log(std::any_cast<std::string>(value)); CatLog::log("\""); break;
			}
		} break;
		case SpecificType::ObjectType:
		{
			 CatLog::log(Tools::makeString(std::any_cast<Reflectable*>(value))); 		
		} break;
		case SpecificType::ContainerType:
		{
			if (containerType == ContainerType::Vector)
			{
				CatLog::log("Vector of ");
				CatLog::log(getContainerItemType().toString());
			}
			else if (containerType == ContainerType::StringMap)
			{
				CatLog::log("Map of string to ");
				CatLog::log(getContainerItemType().toString());
			}
		} break;
	}
}


float CatGenericType::convertToFloat(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<float>(floatType.convertToType(value, valueType));
}


int CatGenericType::convertToInt(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<int>(intType.convertToType(value, valueType));
}


bool CatGenericType::convertToBoolean(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<bool>(boolType.convertToType(value, valueType));
}


std::string CatGenericType::convertToString(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<std::string>(stringType.convertToType(value, valueType));
}


const CatGenericType CatGenericType::intType = CatGenericType(CatType::Int);
const CatGenericType CatGenericType::floatType = CatGenericType(CatType::Float);
const CatGenericType CatGenericType::boolType = CatGenericType(CatType::Bool);
const CatGenericType CatGenericType::stringType = CatGenericType(CatType::String);