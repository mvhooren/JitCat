/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatGenericType.h"
#include "jitcat/ArrayTypeInfo.h"
#include "jitcat/CatLog.h"
#include "jitcat/Configuration.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/StaticMemberFunctionInfo.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/XMLHelper.h"

#include <cassert>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <locale>
#include <string>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatGenericType::CatGenericType(SpecificType specificType, BasicType basicType, TypeInfo* nestedType, Reflection::TypeOwnershipSemantics ownershipSemantics, CatGenericType* pointee, bool writable, bool constant):
	specificType(specificType),
	basicType(basicType),
	writable(writable),
	constant(constant),
	nestedType(nestedType),
	ownershipSemantics(ownershipSemantics)
{
	if (pointee != nullptr)
	{
		pointeeType = std::make_unique<CatGenericType>(*pointee);
	}
}


CatGenericType::CatGenericType(BasicType basicType, bool writable, bool constant):
	specificType(SpecificType::Basic),
	basicType(basicType),
	writable(writable),
	constant(constant),
	nestedType(nullptr),
	ownershipSemantics(TypeOwnershipSemantics::Value)
{
}


CatGenericType::CatGenericType():
	specificType(SpecificType::None),
	basicType(BasicType::None),
	writable(false),
	constant(false),
	nestedType(nullptr),
	ownershipSemantics(TypeOwnershipSemantics::Weak)
{
}


CatGenericType::CatGenericType(const CatGenericType& enumUnderlyingType, Reflection::TypeInfo* enumValuesType, bool writable, bool constant):
	specificType(SpecificType::Enum),
	writable(writable),
	constant(constant),
	nestedType(enumValuesType),
	ownershipSemantics(TypeOwnershipSemantics::Value)
{
	assert(enumUnderlyingType.isBasicType());
	basicType = enumUnderlyingType.basicType;
}


CatGenericType::CatGenericType(TypeInfo* reflectableType, bool writable, bool constant):
	specificType(SpecificType::ReflectableObject),
	basicType(BasicType::None),
	writable(writable),
	constant(constant),
	nestedType(reflectableType),
	ownershipSemantics(TypeOwnershipSemantics::Value)
{
}


CatGenericType::CatGenericType(const CatGenericType& pointee, Reflection::TypeOwnershipSemantics ownershipSemantics, bool isHandle, bool writable, bool constant) :
	specificType(isHandle ? SpecificType::ReflectableHandle : SpecificType::Pointer),
	basicType(BasicType::None),
	writable(writable),
	constant(constant),
	nestedType(nullptr),
	pointeeType(std::make_unique<CatGenericType>(pointee)),
	ownershipSemantics(ownershipSemantics)
{
}


CatGenericType::CatGenericType(const CatGenericType& other):
	specificType(other.specificType),
	basicType(other.basicType),
	writable(other.writable),
	constant(other.constant),
	nestedType(other.nestedType),
	ownershipSemantics(other.ownershipSemantics)
{
	if (other.pointeeType != nullptr)
	{
		pointeeType = std::make_unique<CatGenericType>(*other.pointeeType.get());
	}
}


CatGenericType& CatGenericType::operator=(const CatGenericType& other)
{
	specificType = other.specificType;
	basicType = other.basicType;
	nestedType = other.nestedType;
	ownershipSemantics = other.ownershipSemantics;
	writable = other.writable;
	constant = other.constant;
	if (other.pointeeType != nullptr)
	{
		pointeeType = std::make_unique<CatGenericType>(*other.pointeeType.get());
	}
	else
	{
		pointeeType.reset(nullptr);
	}
	return *this;
}


bool CatGenericType::operator==(const CatGenericType& other) const
{
	return compare(other, true, false);
}


bool CatGenericType::operator!=(const CatGenericType& other) const
{
	return !compare(other, true, false);
}


bool CatGenericType::compare(const CatGenericType& other, bool includeOwnershipSemantics, bool includeIndirection) const
{
	if (!includeIndirection
		&& (specificType == SpecificType::Pointer || specificType == SpecificType::ReflectableHandle
		    || other.specificType == SpecificType::Pointer || other.specificType == SpecificType::ReflectableHandle)
		&& (!includeOwnershipSemantics || ownershipSemantics == other.ownershipSemantics))
	{
		return removeIndirection().compare(other.removeIndirection(), includeOwnershipSemantics, false);
	}
	if ((specificType == other.specificType 
		|| (specificType == SpecificType::ReflectableHandle && other.specificType == SpecificType::Pointer)
		|| (specificType == SpecificType::Pointer && other.specificType == SpecificType::ReflectableHandle))
		&& (!includeOwnershipSemantics || ownershipSemantics == other.ownershipSemantics))
	{
		switch (specificType)
		{
			default:								assert(false);
			case SpecificType::None:				return true;
			case SpecificType::Basic:				return basicType == other.basicType;
			case SpecificType::Enum:				return basicType == other.basicType && nestedType == other.nestedType;
			case SpecificType::ReflectableHandle:
			case SpecificType::Pointer:				return pointeeType->compare(*other.getPointeeType(), includeOwnershipSemantics, includeIndirection);
			case SpecificType::ReflectableObject:	return nestedType == other.nestedType || isNullptrType() || other.isNullptrType();
		}
	}
	else
	{
		return false;
	}
}


bool CatGenericType::isUnknown() const
{
	return specificType == SpecificType::None;
}


bool CatGenericType::isValidType() const
{
	switch (specificType)
	{
		default:
		case SpecificType::None:				return false;
		case SpecificType::Basic:				return basicType != BasicType::None;
		case SpecificType::Enum:				return basicType != BasicType::None && nestedType != nullptr;
		case SpecificType::ReflectableObject:	return nestedType != nullptr;
		case SpecificType::Pointer:
		case SpecificType::ReflectableHandle:	return pointeeType != nullptr && pointeeType->isValidType();
			
	}
}


bool CatGenericType::isBasicType() const
{
	return specificType == SpecificType::Basic 
						   && basicType != BasicType::None
						   && basicType != BasicType::Count;
}


bool CatGenericType::isBoolType() const
{
	return specificType == SpecificType::Basic && basicType == BasicType::Bool;
}


bool CatGenericType::isCharType() const
{
	return specificType == SpecificType::Basic && basicType == BasicType::Char;
}


bool CatGenericType::isUCharType() const
{
	return specificType == SpecificType::Basic && basicType == BasicType::UChar;
}


bool CatGenericType::isIntType() const
{
	return specificType == SpecificType::Basic && basicType == BasicType::Int;
}


bool CatGenericType::isUIntType() const
{
	return specificType == SpecificType::Basic && basicType == BasicType::UInt;
}


bool CatGenericType::isInt64Type() const
{
	return specificType == SpecificType::Basic && basicType == BasicType::Int64;
}


bool jitcat::CatGenericType::isUInt64Type() const
{
	return specificType == SpecificType::Basic && basicType == BasicType::UInt64;
}


bool CatGenericType::isIntegralType() const
{
	return specificType == SpecificType::Basic 
		   && (basicType == BasicType::Int
			   || basicType == BasicType::UInt64);
}


bool CatGenericType::isFloatType() const
{
	return specificType == SpecificType::Basic && basicType == BasicType::Float;
}


bool CatGenericType::isDoubleType() const
{
	return specificType == SpecificType::Basic && basicType == BasicType::Double;
}


bool CatGenericType::isStringType() const
{
	return isStringPtrType() || isStringValueType();
}


bool CatGenericType::isStringPtrType() const
{
	return isPointerType() && *getPointeeType() == CatGenericType::stringType;
}


bool CatGenericType::isStringValueType() const
{
	return compare(CatGenericType::stringType, false, true);
}


bool CatGenericType::isScalarType() const
{
	return specificType == SpecificType::Basic 
						   && (	  basicType == BasicType::Float 
							   || basicType == BasicType::Double 
							   || basicType == BasicType::Char
							   || basicType == BasicType::UChar
							   || basicType == BasicType::Int
							   || basicType == BasicType::UInt
							   || basicType == BasicType::Int64
							   || basicType == BasicType::UInt64);
}


bool CatGenericType::isSignedType() const
{
	return specificType == SpecificType::Basic 
						   && (	  basicType == BasicType::Float 
							   || basicType == BasicType::Double 
							   || basicType == BasicType::Char
							   || basicType == BasicType::Int
							   || basicType == BasicType::Int64);
}


bool CatGenericType::isUnsignedType() const
{
	return specificType == SpecificType::Basic 
						   && (  basicType == BasicType::UChar
							   || basicType == BasicType::UInt
							   || basicType == BasicType::UInt64);
}


bool CatGenericType::isVoidType() const
{
	return specificType == SpecificType::Basic && basicType == BasicType::Void;
}


bool CatGenericType::isEnumType() const
{
	return specificType == SpecificType::Enum;
}


bool CatGenericType::isReflectableObjectType() const
{
	return specificType == SpecificType::ReflectableObject;
}


bool CatGenericType::isReflectableHandleType() const
{
	return specificType == SpecificType::ReflectableHandle;
}


bool CatGenericType::isPointerToReflectableObjectType() const
{
	return specificType == SpecificType::Pointer && pointeeType->isReflectableObjectType();
}


bool CatGenericType::isReflectablePointerOrHandle() const
{
	return (specificType == SpecificType::Pointer && pointeeType->isReflectableObjectType()) 
		    || specificType == SpecificType::ReflectableHandle;
}


bool CatGenericType::isPointerType() const
{
	return specificType == SpecificType::Pointer;
}


bool CatGenericType::isPointerToPointerType() const
{
	return specificType == SpecificType::Pointer && pointeeType->isPointerType();
}


bool CatGenericType::isPointerToHandleType() const
{
	return specificType == SpecificType::Pointer && pointeeType->isReflectableHandleType();
}


bool CatGenericType::isAssignableType() const
{
	return specificType == SpecificType::Pointer
		   && pointeeType->isWritable()
		   && (	  (pointeeType->isBasicType())
			   || (pointeeType->isReflectableObjectType() 
				   && (pointeeType->getObjectType()->canBeAssignedBy(*this)
					   || pointeeType->getOwnershipSemantics() == TypeOwnershipSemantics::Value))
			   || (pointeeType->isEnumType())
			   || (pointeeType->isReflectableHandleType())
			   || (pointeeType->isPointerType() && pointeeType->pointeeType->isReflectableObjectType()));
		
}


bool CatGenericType::isNullptrType() const
{
	return isPointerType() && getPointeeType()->isReflectableObjectType() && getPointeeType()->getObjectType()->getTypeName() == std::string("nullptr");
}


bool CatGenericType::isTriviallyCopyable() const
{
	return (isBasicType() 
		   || (isReflectableObjectType() && nestedType->isTriviallyCopyable())
		   || isEnumType());
}


bool CatGenericType::isWritable() const
{
	return writable || (isPointerToReflectableObjectType() && ownershipSemantics == TypeOwnershipSemantics::Value && pointeeType->isWritable());
}


bool CatGenericType::isConst() const
{
	return constant;
}


void CatGenericType::addDependentType(Reflection::TypeInfo* objectType)
{
	switch (specificType)
	{
		case SpecificType::Basic:				return;
		case SpecificType::ReflectableHandle:
		case SpecificType::Pointer:				pointeeType->addDependentType(objectType); return;
		case SpecificType::Enum:
		case SpecificType::ReflectableObject:	nestedType->addDependentType(objectType); return;
		default:								assert(false); return;
	}
}


bool CatGenericType::isDependentOn(Reflection::TypeInfo* objectType) const
{
	switch (specificType)
	{
		case SpecificType::Enum:
		case SpecificType::Basic:				return false;
		case SpecificType::ReflectableHandle:
		case SpecificType::Pointer:				return pointeeType->isDependentOn(objectType);
		case SpecificType::ReflectableObject:	return nestedType == objectType;
		default:								assert(false); return false;
	}
}


const CatGenericType& CatGenericType::getUnderlyingEnumType() const
{
	assert(isEnumType());
	return getBasicType(basicType);
}


CatGenericType CatGenericType::copyWithFlags(bool writable, bool constant) const
{
	return CatGenericType(specificType, basicType, nestedType, ownershipSemantics, pointeeType.get(), writable, constant);
}


CatGenericType CatGenericType::toUnmodified() const
{
	return CatGenericType(specificType, basicType, nestedType, ownershipSemantics, pointeeType.get(), false, false);
}


CatGenericType CatGenericType::toUnwritable() const
{
	return CatGenericType(specificType, basicType, nestedType, ownershipSemantics, pointeeType.get(), false, constant);
}


CatGenericType CatGenericType::toWritable() const
{
	return CatGenericType(specificType, basicType, nestedType, ownershipSemantics, pointeeType.get(), true, constant);
}


CatGenericType CatGenericType::toValueOwnership() const
{
	return CatGenericType(specificType, basicType, nestedType, TypeOwnershipSemantics::Value, pointeeType.get(), true, constant);
}


CatGenericType CatGenericType::toChangedOwnership(Reflection::TypeOwnershipSemantics ownershipSemantics_) const
{
	return CatGenericType(specificType, basicType, nestedType, ownershipSemantics_, pointeeType.get(), true, constant);
}


CatGenericType CatGenericType::toPointer(TypeOwnershipSemantics ownershipSemantics, bool writable, bool constant) const
{
	return CatGenericType(*this, ownershipSemantics, false, writable, constant);
}


CatGenericType CatGenericType::toHandle(Reflection::TypeOwnershipSemantics ownershipSemantics, bool writable, bool constant) const
{
	return CatGenericType(*this, ownershipSemantics, true, writable, constant);
}


CatGenericType CatGenericType::convertPointerToHandle() const
{
	assert(specificType == SpecificType::Pointer);
	return CatGenericType(SpecificType::ReflectableHandle, basicType, nestedType, ownershipSemantics, pointeeType.get(), writable, constant);
}


CatGenericType jitcat::CatGenericType::toArray() const
{
	return createArrayType(*this, true, false);
}


const CatGenericType& CatGenericType::removeIndirection() const
{
	const CatGenericType* currentType = this;
	while (currentType->isPointerType() || currentType->isReflectableHandleType())
	{
		currentType = currentType->getPointeeType();
	}
	return *currentType;
}


const CatGenericType& CatGenericType::removeIndirection(int& levelsOfIndirectionRemoved) const
{
	levelsOfIndirectionRemoved = 0;
	const CatGenericType* currentType = this;
	while (currentType->isPointerType() || currentType->isReflectableHandleType())
	{
		levelsOfIndirectionRemoved++;
		currentType = currentType->getPointeeType();
	}
	return *currentType;
}


IndirectionConversionMode CatGenericType::getIndirectionConversion(const CatGenericType& other) const
{
	int fromIndirection = 0;
	int toIndirection = 0;
	if (removeIndirection(toIndirection).compare(other.removeIndirection(fromIndirection), false, false))
	{
		if (toIndirection == fromIndirection)
		{
			if (toIndirection == 0 && !isConstructible())
			{
				return IndirectionConversionMode::ErrorNotCopyConstructible;
			}
			else
			{
				return IndirectionConversionMode::None;
			}
		}
		switch (toIndirection)
		{
			case 0:
			{
				if (isCopyConstructible())
				{
					switch (fromIndirection)
					{
						case 1:		return IndirectionConversionMode::DereferencePointer;
						case 2:		return IndirectionConversionMode::DereferencePointerToPointerTwice;
						default:	return IndirectionConversionMode::ErrorTooMuchIndirection;
					}
				}
				else
				{
					return IndirectionConversionMode::ErrorNotCopyConstructible;
				}
			
			} break;
			case 1:
			{
				switch (fromIndirection)
				{
					case 0:		return IndirectionConversionMode::AddressOfValue;
					case 2:		return IndirectionConversionMode::DereferencePointerToPointer;
					default:	return IndirectionConversionMode::ErrorTooMuchIndirection;
				}
			} break;
			case 2:
			{
				switch (fromIndirection)
				{
					case 1:		return IndirectionConversionMode::AddressOfPointer;			
					default:	return IndirectionConversionMode::ErrorTooMuchIndirection;
				}
			} break;
			default: return IndirectionConversionMode::ErrorTooMuchIndirection;
		}
	}
	else
	{
		return IndirectionConversionMode::ErrorTypeMismatch;
	}
}


std::any CatGenericType::doIndirectionConversion(std::any& value, IndirectionConversionMode mode) const
{
	const CatGenericType& typeWithoutIndirection = removeIndirection();
	const TypeCaster* typeCaster = typeWithoutIndirection.getTypeCaster();
	switch (mode)
	{
		default:															return value;
		case IndirectionConversionMode::AddressOfPointer:					return typeCaster->getAddressOfPointer(value);
		case IndirectionConversionMode::AddressOfValue:						return typeCaster->getAddressOfValue(value);
		case IndirectionConversionMode::DereferencePointer:
		{
			if (!typeCaster->isNullPtr(value))
			{
				return typeCaster->getValueOfPointer(value);
			}
			else
			{
				return typeWithoutIndirection.createDefault();
			}
		}
		case IndirectionConversionMode::DereferencePointerToPointer:
		{
			if (!typeCaster->isNullPtrPtr(value))
			{
				return typeCaster->getValueOfPointerToPointer(value);
			}
			else
			{
				return typeWithoutIndirection.toPointer().createDefault();
			}
		}
		case IndirectionConversionMode::DereferencePointerToPointerTwice:	
		{
			if (!typeCaster->isNullPtrPtr(value))
			{
				std::any ptr = typeCaster->getValueOfPointerToPointer(value);
				if (!typeCaster->isNullPtr(ptr))
				{
					return typeCaster->getValueOfPointer(ptr);
				}
			}
			return typeWithoutIndirection.createDefault();
		}
	}
}


const char* CatGenericType::getObjectTypeName() const
{
	if ((specificType == SpecificType::ReflectableObject
		 || specificType == SpecificType::Enum)
		&& nestedType != nullptr)
	{
		return nestedType->getTypeName();
	}
	else
	{
		return nullptr;
	}
}


InfixOperatorResultInfo CatGenericType::getInfixOperatorResultInfo(CatInfixOperatorType oper, const CatGenericType& rightType)
{
	InfixOperatorResultInfo resultInfo;
	resultInfo.setIsOverloaded(false);
	if (!rightType.isValidType())
	{
		resultInfo.setResultType(rightType);
		return resultInfo;
	}
	else if (!isValidType())
	{
		resultInfo.setResultType(*this);
		return resultInfo;
	}
	else if (isBasicType()
			 && rightType.isBasicType())
	{
		switch (oper)
		{
			default:
			case CatInfixOperatorType::Plus:			
				if (isStringType() || rightType.isStringType())
				{
					resultInfo.setResultType(CatGenericType::stringWeakPtrType);
					return resultInfo;
				}
				//Intentional lack of break
			case CatInfixOperatorType::Minus:
			case CatInfixOperatorType::Multiply:
			case CatInfixOperatorType::Divide:
				if (isScalarType() && rightType.isScalarType())
				{
					resultInfo.setResultType(CatGenericType(getWidestType(basicType, rightType.basicType)));
					return resultInfo;
				}
				break;
			case CatInfixOperatorType::Modulo:
				if (isScalarType() && rightType.isScalarType())
				{
					resultInfo.setResultType(CatGenericType(basicType));
					return resultInfo;
				}
				break;
			case CatInfixOperatorType::Greater:
			case CatInfixOperatorType::Smaller:
			case CatInfixOperatorType::GreaterOrEqual:
			case CatInfixOperatorType::SmallerOrEqual:
				if (isScalarType() && rightType.isScalarType())
				{
					resultInfo.setResultType(CatGenericType::boolType);
					return resultInfo;
				}
				break;
			case CatInfixOperatorType::Equals:
			case CatInfixOperatorType::NotEquals:
				if (*this == rightType
					|| (isScalarType() && rightType.isScalarType()))
				{
					resultInfo.setResultType(CatGenericType::boolType);
					return resultInfo;
				}
				break;
			case CatInfixOperatorType::LogicalAnd:
			case CatInfixOperatorType::LogicalOr:
				if (isBoolType() && rightType.isBoolType())
				{
					resultInfo.setResultType(CatGenericType::boolType);
					return resultInfo;
				}
				break;
		}
	}
	else 
	{
		//First check if there is a member function overloading this operator.
		MemberFunctionInfo* memberFunctionInfo = nullptr;
		if (isReflectablePointerOrHandle())
		{
			SearchFunctionSignature signature(::toString(oper), {rightType});
			memberFunctionInfo = pointeeType->nestedType->getMemberFunctionInfo(&signature);
		}
		else if (isReflectableObjectType())
		{
			SearchFunctionSignature signature(::toString(oper), {rightType});
			memberFunctionInfo = nestedType->getMemberFunctionInfo(&signature);
		}
		resultInfo.setIsOverloaded(true);
		if (memberFunctionInfo != nullptr)
		{
			resultInfo.setResultType(memberFunctionInfo->getReturnType());
			return resultInfo;
		}
		else
		{
			//If there is no member function, check for a static member function in either type.
			SearchFunctionSignature staticSignature(::toString(oper), {*this, rightType});
			StaticFunctionInfo* staticMemberFunctionInfo = nullptr;
			if (isReflectablePointerOrHandle())
			{
				staticMemberFunctionInfo = pointeeType->nestedType->getStaticMemberFunctionInfo(&staticSignature);
				if (staticMemberFunctionInfo != nullptr)
				{
					resultInfo.setStaticOverloadType(pointeeType->nestedType);
				}
			}
			else if (isReflectableObjectType())
			{
				staticMemberFunctionInfo = nestedType->getStaticMemberFunctionInfo(&staticSignature);
				if (staticMemberFunctionInfo != nullptr)
				{
					resultInfo.setStaticOverloadType(nestedType);
				}			
			}
			if (staticMemberFunctionInfo == nullptr)
			{
				if (rightType.isReflectablePointerOrHandle())
				{
					staticMemberFunctionInfo = rightType.getPointeeType()->nestedType->getStaticMemberFunctionInfo(&staticSignature);
					if (staticMemberFunctionInfo != nullptr)
					{
						resultInfo.setStaticOverloadType(rightType.getPointeeType()->nestedType);
					}
				}
				else if (rightType.isReflectableObjectType())
				{
					staticMemberFunctionInfo = rightType.nestedType->getStaticMemberFunctionInfo(&staticSignature);
					if (staticMemberFunctionInfo != nullptr)
					{
						resultInfo.setStaticOverloadType(rightType.nestedType);
					}
				}
			}

			if (staticMemberFunctionInfo != nullptr)
			{
				resultInfo.setResultType(staticMemberFunctionInfo->getReturnType());
				return resultInfo;
			}
		}
	}

	return resultInfo;
}


std::string CatGenericType::toString() const
{
	switch (specificType)
	{
		default:								return "unknown";
		case SpecificType::Basic:				return toString(basicType);
		case SpecificType::Enum:
		case SpecificType::ReflectableObject:	return nestedType->getTypeName();
		case SpecificType::Pointer:				return Tools::append(pointeeType->toString(), "*");
		case SpecificType::ReflectableHandle:	return Tools::append(pointeeType->toString(), "#");
	}
}


CatGenericType* CatGenericType::getPointeeType() const
{
	return pointeeType.get();
}


void CatGenericType::setPointeeType(std::unique_ptr<CatGenericType> pointee)
{
	pointeeType = std::move(pointee);
}


TypeInfo* CatGenericType::getObjectType() const
{
	return nestedType;
}


Reflection::TypeOwnershipSemantics CatGenericType::getOwnershipSemantics() const
{
	return ownershipSemantics;
}


void CatGenericType::setOwnershipSemantics(Reflection::TypeOwnershipSemantics semantics)
{
	ownershipSemantics = semantics;
}


std::any CatGenericType::createAnyOfType(uintptr_t pointer)
{
	switch (specificType)
	{
		case SpecificType::ReflectableHandle:
		{
			return pointeeType->getObjectType()->getTypeCaster()->castFromRawPointer(pointer);
		} break;
		case SpecificType::Pointer:
		{
			switch (pointeeType->specificType)
			{
				case SpecificType::Basic:
				{
					switch (pointeeType->basicType)
					{
						case BasicType::Char:	return std::any(reinterpret_cast<char*>(pointer));
						case BasicType::UChar:	return std::any(reinterpret_cast<unsigned char*>(pointer));
						case BasicType::Int:	return std::any(reinterpret_cast<int*>(pointer));
						case BasicType::UInt:	return std::any(reinterpret_cast<unsigned int*>(pointer));
						case BasicType::Int64:	return std::any(reinterpret_cast<int64_t*>(pointer));
						case BasicType::UInt64:	return std::any(reinterpret_cast<uint64_t*>(pointer));
						case BasicType::Float:	return std::any(reinterpret_cast<float*>(pointer));
						case BasicType::Double:	return std::any(reinterpret_cast<double*>(pointer));
						case BasicType::Bool:	return std::any(reinterpret_cast<bool*>(pointer));
						case BasicType::Void:	return std::any();
						default:				assert(!isValidBasicType(pointeeType->basicType));
					}
				} break;
				case SpecificType::Enum:
				case SpecificType::ReflectableObject:
				{
					return pointeeType->getObjectType()->getTypeCaster()->castFromRawPointer(pointer);
				} break;
				case SpecificType::ReflectableHandle:
				{
					return std::any(reinterpret_cast<ReflectableHandle*>(pointer));
				}
				case SpecificType::Pointer:
				{
					switch (pointeeType->pointeeType->specificType)
					{
						case SpecificType::ReflectableObject:
						{
							return pointeeType->createFromRawPointer(pointer);
						}
						default:
						{
							assert(false);
						}
					}
				}
				default:	assert(false);
			}
		} break;
		default:
		{
			assert(false);
		} break;
	}
	return std::any();
}


std::any CatGenericType::createAnyOfTypeAt(uintptr_t pointer)
{
	switch (specificType)
	{
		case SpecificType::ReflectableHandle:
		{
			return reinterpret_cast<ReflectableHandle*>(pointer)->get();
		} break;
		case SpecificType::Basic:
		{
			switch (basicType)
			{
				case BasicType::Char:	return std::any(*reinterpret_cast<char*>(pointer));
				case BasicType::UChar:	return std::any(*reinterpret_cast<unsigned char*>(pointer));
				case BasicType::Int:	return std::any(*reinterpret_cast<int*>(pointer));
				case BasicType::UInt:	return std::any(*reinterpret_cast<unsigned int*>(pointer));
				case BasicType::Int64:	return std::any(*reinterpret_cast<int64_t*>(pointer));
				case BasicType::UInt64:	return std::any(*reinterpret_cast<uint64_t*>(pointer));
				case BasicType::Float:	return std::any(*reinterpret_cast<float*>(pointer));
				case BasicType::Double:	return std::any(*reinterpret_cast<double*>(pointer));
				case BasicType::Bool:	return std::any(*reinterpret_cast<bool*>(pointer));
				default: assert(false);
			}
		} break;
		case SpecificType::Enum:
		case SpecificType::ReflectableObject:
		{
			return createFromRawPointer(pointer);
		}
		case SpecificType::Pointer:
		{
			switch (pointeeType->specificType)
			{
				case SpecificType::Basic:
				{
					switch (pointeeType->basicType)
					{
						case BasicType::Char:	return std::any(*reinterpret_cast<char**>(pointer));
						case BasicType::UChar:	return std::any(*reinterpret_cast<unsigned char**>(pointer));
						case BasicType::Int:	return std::any(*reinterpret_cast<int**>(pointer));
						case BasicType::UInt:	return std::any(*reinterpret_cast<unsigned int**>(pointer));
						case BasicType::Int64:	return std::any(*reinterpret_cast<int64_t**>(pointer));
						case BasicType::UInt64:	return std::any(*reinterpret_cast<uint64_t**>(pointer));
						case BasicType::Float:	return std::any(*reinterpret_cast<float**>(pointer));
						case BasicType::Double:	return std::any(*reinterpret_cast<double**>(pointer));
						case BasicType::Bool:	return std::any(*reinterpret_cast<bool**>(pointer));
						default: assert(false);
					}
				} break;
				case SpecificType::ReflectableObject:
				{
					return createFromRawPointer(pointer);
				} break;
				case SpecificType::ReflectableHandle:
				{
					return createFromRawPointer(reinterpret_cast<uintptr_t>((*reinterpret_cast<ReflectableHandle**>(pointer))->get()));
				}
				default: assert(false);
			}
		} break;
		default:
		{
			assert(false);
		} break;
	}
	return std::any();
}


std::any CatGenericType::createDefault() const
{
	switch (specificType)
	{
		case SpecificType::Basic:
		{
			switch (basicType)
			{
				case BasicType::Char:	return (char)0;
				case BasicType::UChar:	return (unsigned char)0;
				case BasicType::Int:	return 0;
				case BasicType::UInt:	return (unsigned int)0;
				case BasicType::Int64:	return (int64_t)0;
				case BasicType::UInt64:	return (uint64_t)0;
				case BasicType::Float:	return 0.0f;
				case BasicType::Double:	return 0.0;
				case BasicType::Bool:	return false;
				case BasicType::Void:	return std::any();
				default:				assert(false);
			}
		} break;
		case SpecificType::Enum:
		case SpecificType::ReflectableObject:
		{
			return construct();
		} break;
		case SpecificType::Pointer:
		{
			switch (pointeeType->specificType)
			{
				case SpecificType::Basic:
				{
					switch (pointeeType->basicType)
					{
						case BasicType::Char:	return (char*)nullptr;
						case BasicType::UChar:	return (unsigned char*)nullptr;
						case BasicType::Int:	return (int*)nullptr;
						case BasicType::UInt:	return (unsigned int*)nullptr;
						case BasicType::Int64:	return (int64_t*)nullptr;
						case BasicType::UInt64:	return (uint64_t*)nullptr;
						case BasicType::Float:	return (float*)nullptr;
						case BasicType::Double:	return (double*)nullptr;
						case BasicType::Bool:	return (bool*)nullptr;
						case BasicType::Void:	return (void*)nullptr;
						default:				assert(false);
					}
				}
				case SpecificType::ReflectableObject:
				{
					return createNullPtr();
				}
				case SpecificType::Pointer:
				{
					if (pointeeType->pointeeType->specificType == SpecificType::ReflectableObject)
					{
						return createNullPtr();
					}
					else
					{
						assert(false);
					}
				}
				case SpecificType::ReflectableHandle:
				{
					return (ReflectableHandle*)(nullptr);
				}
				default: assert(false);
			}
		}
		case SpecificType::ReflectableHandle:
		{
			return createNullPtr();
		}
		default: assert(false);
	}
	assert(false);
	return std::any();
}


std::size_t CatGenericType::getTypeSize() const
{
	switch (specificType)
	{
		case SpecificType::Basic:
		case SpecificType::Enum:
			switch (basicType)
			{
				case BasicType::Bool:	return sizeof(bool);
				case BasicType::Float:	return sizeof(float);
				case BasicType::Double:	return sizeof(double);
				case BasicType::Char:	return sizeof(char);
				case BasicType::UChar:	return sizeof(unsigned char);
				case BasicType::Int:	return sizeof(int);
				case BasicType::UInt:	return sizeof(unsigned int);
				case BasicType::Int64:	return sizeof(int64_t);
				case BasicType::UInt64:	return sizeof(uint64_t);
				case BasicType::Void:	return 0;
				default:	assert(false);
			}
			break;
		case SpecificType::Pointer:				
		{
			if (ownershipSemantics == TypeOwnershipSemantics::Value)
			{
				return getPointeeType()->getTypeSize();
			}
			else
			{
				return sizeof(uintptr_t);
			}
		}
		case SpecificType::ReflectableHandle:	return sizeof(ReflectableHandle);
		case SpecificType::ReflectableObject:	return nestedType->getTypeSize();
		default: assert(false);
	}
	assert(false);
	return 0;
}


std::any CatGenericType::convertToType(std::any& value, const CatGenericType& valueType) const
{
	if (this->operator==(valueType))
	{
		return value;
	}
	else if (isBasicType() && valueType.isBasicType())
	{
		switch(basicType)
		{
			case BasicType::Char:
			{
				switch (valueType.basicType)
				{
					case BasicType::UChar:	return (char)std::any_cast<unsigned char>(value);
					case BasicType::Int:	return (char)std::any_cast<int>(value);
					case BasicType::Int64:	return (char)std::any_cast<int64_t>(value);
					case BasicType::UInt64:	return (char)std::any_cast<uint64_t>(value);
					case BasicType::Float:	return (char)std::any_cast<float>(value);
					case BasicType::Double:	return (char)std::any_cast<double>(value);
					case BasicType::Bool:	return (char)std::any_cast<bool>(value) ? 1 : 0;
					default: assert(false);
				}
			} break;
			case BasicType::UChar:
			{
				switch (valueType.basicType)
				{
					case BasicType::Char:	return (unsigned char)std::any_cast<char>(value);
					case BasicType::Int:	return (unsigned char)std::any_cast<int>(value);
					case BasicType::Int64:	return (unsigned char)std::any_cast<int64_t>(value);
					case BasicType::UInt64:	return (unsigned char)std::any_cast<uint64_t>(value);
					case BasicType::Float:	return (unsigned char)std::any_cast<float>(value);
					case BasicType::Double:	return (unsigned char)std::any_cast<double>(value);
					case BasicType::Bool:	return (unsigned char)std::any_cast<bool>(value) ? 1 : 0;
					default: assert(false);
				}
			} break;
			case BasicType::Int:
			{
				switch (valueType.basicType)
				{
					case BasicType::Char:	return (int)std::any_cast<char>(value);
					case BasicType::UChar:	return (int)std::any_cast<unsigned char>(value);
					case BasicType::UInt:	return (int)std::any_cast<unsigned int>(value);
					case BasicType::Int64:	return (int)std::any_cast<int64_t>(value);
					case BasicType::UInt64:	return (int)std::any_cast<uint64_t>(value);
					case BasicType::Float:	return (int)std::any_cast<float>(value);
					case BasicType::Double:	return (int)std::any_cast<double>(value);
					case BasicType::Bool:	return std::any_cast<bool>(value) ? 1 : 0;
					default: assert(false);
				}
			} break;
			case BasicType::UInt64:
			{
				switch (valueType.basicType)
				{
					case BasicType::Char:	return (uint64_t)std::any_cast<char>(value);
					case BasicType::UChar:	return (uint64_t)std::any_cast<unsigned char>(value);
					case BasicType::Int:	return (uint64_t)std::any_cast<int>(value);
					case BasicType::UInt:	return (uint64_t)std::any_cast<unsigned int>(value);
					case BasicType::Int64:	return (uint64_t)std::any_cast<int64_t>(value);
					case BasicType::Float:	return (uint64_t)std::any_cast<float>(value);
					case BasicType::Double:	return (uint64_t)std::any_cast<double>(value);
					case BasicType::Bool:	return (uint64_t)std::any_cast<bool>(value) ? 1 : 0;
					default: assert(false);
				}
			} break;
			case BasicType::Float:
			{
				switch (valueType.basicType)
				{
					case BasicType::Char:	return (float)std::any_cast<char>(value);
					case BasicType::UChar:	return (float)std::any_cast<unsigned char>(value);
					case BasicType::Int:	return (float)std::any_cast<int>(value);
					case BasicType::UInt:	return (float)std::any_cast<unsigned int>(value);
					case BasicType::Int64:	return (float)std::any_cast<int64_t>(value);
					case BasicType::UInt64:	return (float)std::any_cast<uint64_t>(value);
					case BasicType::Double:	return (float)std::any_cast<double>(value);
					case BasicType::Bool:	return std::any_cast<bool>(value) ? 1.0f : 0.0f;
					default: assert(false);
				}
			} break;
			case BasicType::Double:
			{
				switch (valueType.basicType)
				{
					case BasicType::Char:	return (double)std::any_cast<char>(value);
					case BasicType::UChar:	return (double)std::any_cast<unsigned char>(value);
					case BasicType::Int:	return (double)std::any_cast<int>(value);
					case BasicType::UInt:	return (double)std::any_cast<unsigned int>(value);
					case BasicType::Int64:	return (double)std::any_cast<int64_t>(value);
					case BasicType::UInt64:	return (double)std::any_cast<uint64_t>(value);
					case BasicType::Float:	return (double)std::any_cast<float>(value);
					case BasicType::Bool:	return std::any_cast<bool>(value) ? 1.0 : 0.0;
					default: assert(false);
				}
			} break;
			case BasicType::Bool:
			{
				switch (valueType.basicType)
				{
					case BasicType::Char:	return std::any_cast<char>(value) > 0;
					case BasicType::UChar:	return std::any_cast<unsigned char>(value) > 0;
					case BasicType::Int:	return std::any_cast<int>(value) > 0;
					case BasicType::UInt:	return std::any_cast<unsigned int>(value) > 0;
					case BasicType::Int64:	return std::any_cast<int64_t>(value) > 0;
					case BasicType::UInt64:	return std::any_cast<uint64_t>(value) > 0;
					case BasicType::Double:	return std::any_cast<double>(value) > 0.0;
					case BasicType::Float:	return std::any_cast<float>(value) > 0.0f;
					default: assert(false);
				}
			} break;
			default: assert(false);
		}
	}
	else if (isStringValueType() && valueType.isBasicType())
	{
		switch (valueType.basicType)
		{
			case BasicType::Char:
			{
				Configuration::CatStringOStream ss;
				ss << std::any_cast<char>(value);
				return Configuration::CatString(ss.str());
			}
			case BasicType::UChar:
			{
				Configuration::CatStringOStream ss;
				ss << std::any_cast<unsigned char>(value);
				return Configuration::CatString(ss.str());
			}
			case BasicType::Int:
			{
				Configuration::CatStringOStream ss;
				ss << std::any_cast<int>(value);
				return Configuration::CatString(ss.str());
			}
			case BasicType::UInt:
			{
				Configuration::CatStringOStream ss;
				ss << std::any_cast<unsigned int>(value);
				return Configuration::CatString(ss.str());
			}
			case BasicType::Int64:
			{
				Configuration::CatStringOStream ss;
				ss << std::any_cast<int64_t>(value);
				return Configuration::CatString(ss.str());
			}
			case BasicType::UInt64:
			{
				Configuration::CatStringOStream ss;
				ss << std::any_cast<uint64_t>(value);
				return Configuration::CatString(ss.str());
			}
			case BasicType::Double:
			{
				Configuration::CatStringOStream ss;
				ss.imbue(Configuration::localeForStringConversions);
				ss << std::any_cast<double>(value);
				return Configuration::CatString(ss.str());
			}
			case BasicType::Float:
			{
				Configuration::CatStringOStream ss;
				ss.imbue(Configuration::localeForStringConversions);
				ss << std::any_cast<float>(value);
				return Configuration::CatString(ss.str());
			}
			case BasicType::Bool:		
			{
				return std::any_cast<bool>(value) ? Tools::StringConstants<Configuration::CatString>::oneStr : Tools::StringConstants<Configuration::CatString>::zeroStr;
			}
			default: assert(false);
		}
	}
	else if (isBasicType() && valueType.isStringValueType())
	{
		switch(basicType)
		{
			case BasicType::Char:	return (char)Tools::StringConstants<Configuration::CatString>::stringToInt(std::any_cast<Configuration::CatString>(value));
			case BasicType::Int:	return Tools::StringConstants<Configuration::CatString>::stringToInt(std::any_cast<Configuration::CatString>(value));
			case BasicType::Int64:	return Tools::StringConstants<Configuration::CatString>::stringToInt64(std::any_cast<Configuration::CatString>(value));
			case BasicType::UChar:	return (unsigned char)Tools::StringConstants<Configuration::CatString>::stringToUInt64(std::any_cast<Configuration::CatString>(value));
			case BasicType::UInt:	return (unsigned int)Tools::StringConstants<Configuration::CatString>::stringToUInt64(std::any_cast<Configuration::CatString>(value));
			case BasicType::UInt64:	return Tools::StringConstants<Configuration::CatString>::stringToUInt64(std::any_cast<Configuration::CatString>(value));
			case BasicType::Float:	return Tools::StringConstants<Configuration::CatString>::stringToFloat(std::any_cast<Configuration::CatString>(value));
			case BasicType::Double:	return Tools::StringConstants<Configuration::CatString>::stringToDouble(std::any_cast<Configuration::CatString>(value));
			case BasicType::Bool:
			{
				Configuration::CatString strValue = std::any_cast<Configuration::CatString>(value);
				return  strValue == Tools::StringConstants<Configuration::CatString>::trueStr || Tools::StringConstants<Configuration::CatString>::stringToInt(strValue) > 0;
			}
			default: assert(false);
		}
	}
	else if (isBasicType() && valueType.isStringPtrType())
	{
		switch(basicType)
		{
			case BasicType::Char:	return (char)Tools::StringConstants<Configuration::CatString>::stringToInt(*std::any_cast<Configuration::CatString*>(value));
			case BasicType::Int:	return Tools::StringConstants<Configuration::CatString>::stringToInt(*std::any_cast<Configuration::CatString*>(value));
			case BasicType::Int64:	return Tools::StringConstants<Configuration::CatString>::stringToInt64(*std::any_cast<Configuration::CatString*>(value));
			case BasicType::UChar:	return (unsigned char)Tools::StringConstants<Configuration::CatString>::stringToUInt64(*std::any_cast<Configuration::CatString*>(value));
			case BasicType::UInt:	return (unsigned int)Tools::StringConstants<Configuration::CatString>::stringToUInt64(*std::any_cast<Configuration::CatString*>(value));
			case BasicType::UInt64:	return Tools::StringConstants<Configuration::CatString>::stringToUInt64(*std::any_cast<Configuration::CatString*>(value));
			case BasicType::Float:	return Tools::StringConstants<Configuration::CatString>::stringToFloat(*std::any_cast<Configuration::CatString*>(value));
			case BasicType::Double:	return Tools::StringConstants<Configuration::CatString>::stringToDouble(*std::any_cast<Configuration::CatString*>(value));
			case BasicType::Bool:
			{
				Configuration::CatString* strValue = std::any_cast<Configuration::CatString*>(value);
				return  *strValue == Tools::StringConstants<Configuration::CatString>::trueStr || Tools::StringConstants<Configuration::CatString>::stringToInt(*strValue) > 0;
			}
			default: assert(false);
		}
	}
	assert(false);
	return valueType.createDefault();
}


std::any CatGenericType::toUnderlyingType(std::any enumValue) const
{
	assert(isEnumType());
	const unsigned char* bufferAddress = nullptr;
	std::size_t bufferSize = 0;
	//Get an unsigned char pointer to the enum value
	toBuffer(getTypeCaster()->getAddressOfValue(enumValue), bufferAddress, bufferSize);
	//Get the underlying type of the enum
	const CatGenericType& underlyingType = getUnderlyingEnumType();
	//Cast the unsigned char pointer to a pointer to the underlying type
	std::any enumTypePtr = underlyingType.getTypeCaster()->castFromRawPointer(reinterpret_cast<uintptr_t>(bufferAddress));
	//Get the value pointed to by the enumTypePtr.
	std::any underlyingValue = underlyingType.getTypeCaster()->getValueOfPointer(enumTypePtr);
	return underlyingValue;
}


void CatGenericType::printValue(std::any& value)
{
	switch (specificType)
	{
		case SpecificType::Basic:
		{
			switch (basicType)
			{
				case BasicType::Char:	CatLog::log(std::any_cast<char>(value)); break;
				case BasicType::UChar:	CatLog::log(std::any_cast<unsigned char>(value)); break;
				case BasicType::Int:	CatLog::log(std::any_cast<int>(value)); break;
				case BasicType::UInt:	CatLog::log(std::any_cast<unsigned int>(value)); break;
				case BasicType::Int64:	CatLog::log(std::any_cast<int64_t>(value)); break;
				case BasicType::UInt64:	CatLog::log(std::any_cast<uint64_t>(value)); break;
				case BasicType::Float:	CatLog::log(std::any_cast<float>(value)); break;
				case BasicType::Double:	CatLog::log(std::any_cast<double>(value)); break;
				case BasicType::Bool:	CatLog::log(std::any_cast<bool>(value) ? Tools::StringConstants<Configuration::CatString>::trueStr : Tools::StringConstants<Configuration::CatString>::falseStr); break;
				default: assert(false);
			}
		} break;
		case SpecificType::Enum:
		{
			const unsigned char* enumPtr = nullptr;
			std::size_t bufferSize = 0;
			nestedType->getTypeCaster()->toBuffer(value, enumPtr, bufferSize);
			switch (basicType)
			{
				case BasicType::Char:	CatLog::log(*reinterpret_cast<const char*>(enumPtr)); break;
				case BasicType::UChar:	CatLog::log(*reinterpret_cast<const unsigned char*>(enumPtr)); break;
				case BasicType::Int:	CatLog::log(*reinterpret_cast<const unsigned int*>(enumPtr)); break;
				case BasicType::UInt:	CatLog::log(*reinterpret_cast<const unsigned int*>(enumPtr)); break;
				case BasicType::Int64:	CatLog::log(*reinterpret_cast<const uint64_t*>(enumPtr)); break;
				case BasicType::UInt64:	CatLog::log(*reinterpret_cast<const uint64_t*>(enumPtr)); break;
				case BasicType::Double:	CatLog::log(*reinterpret_cast<const double*>(enumPtr)); break;
				case BasicType::Float:	CatLog::log(*reinterpret_cast<const float*>(enumPtr)); break;
				case BasicType::Bool:	CatLog::log(*reinterpret_cast<const bool*>(enumPtr)); break;
				default: assert(false);
			}
			
		} break;
		case SpecificType::ReflectableObject:
		case SpecificType::Pointer:
		{
			if (isStringValueType())
			{
				CatLog::log("\""); CatLog::log(std::any_cast<Configuration::CatString>(value)); CatLog::log("\"");
			}
			else if (isStringPtrType())
			{
				CatLog::log("\""); CatLog::log(*std::any_cast<Configuration::CatString*>(value)); CatLog::log("\"");
			}
			else
			{
				CatLog::log(Tools::makeString(getRawPointer(value))); 		
			}
		} break;
		default: assert(false);
	}
}


float CatGenericType::convertToFloat(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<float>(floatType.convertToType(value, valueType));
}


double CatGenericType::convertToDouble(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<double>(doubleType.convertToType(value, valueType));
}


char CatGenericType::convertToChar(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<char>(charType.convertToType(value, valueType));
}


unsigned char CatGenericType::convertToUChar(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<unsigned char>(uCharType.convertToType(value, valueType));
}


int CatGenericType::convertToInt(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<int>(intType.convertToType(value, valueType));
}


unsigned int CatGenericType::convertToUInt(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<unsigned int>(uIntType.convertToType(value, valueType));
}


int64_t CatGenericType::convertToInt64(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<int64_t>(int64Type.convertToType(value, valueType));
}


uint64_t CatGenericType::convertToUInt64(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<uint64_t>(uInt64Type.convertToType(value, valueType));
}


bool CatGenericType::convertToBoolean(std::any value, const CatGenericType& valueType)
{
	return std::any_cast<bool>(boolType.convertToType(value, valueType));
}


Configuration::CatString CatGenericType::convertToString(std::any value, const CatGenericType& valueType)
{
	if (valueType.isStringType())
	{
		return *std::any_cast<Configuration::CatString*>(value);
	}
	else if (valueType == CatGenericType::stringType)
	{
		return std::any_cast<Configuration::CatString>(value);
	}
	else
	{
		return std::any_cast<Configuration::CatString>(stringType.convertToType(value, valueType));
	}
}


CatGenericType CatGenericType::readFromXML(std::ifstream& xmlFile, const std::string& closingTag, std::map<std::string, TypeInfo*>& typeInfos)
{
	SpecificType specificType = SpecificType::None;
	BasicType basicType = BasicType::None;
	std::string objectTypeName = "";
	std::string containerItemTypeName = "";
	bool writable = false;
	bool constant = false;
	while (true)
	{
		XMLLineType tagType;
		std::string contents;
		std::string tagName = XMLHelper::readXMLLine(xmlFile, tagType, contents);
		if (tagType == XMLLineType::OpenCloseWithContent)
		{
			if (tagName == "Type")
			{
				specificType = toSpecificType(contents.c_str());
			}
			else if (tagName == "BasicType")
			{
				basicType = toBasicType(contents.c_str());
			}
			else if (tagName == "ObjectTypeName")
			{
				objectTypeName = contents;
			}
			else
			{
				return CatGenericType::unknownType;
			}
		}
		else if (tagType == XMLLineType::SelfClosingTag)
		{
			if (tagName == "const")
			{
				constant = true;
			}
			else if (tagName == "writable")
			{
				writable = true;
			}
			else
			{
				return CatGenericType::unknownType;
			}
		}
		else if (tagType == XMLLineType::CloseTag && tagName == closingTag)
		{
			switch (specificType)
			{
				case SpecificType::Basic:
					if (basicType != BasicType::None)
					{
						return CatGenericType(basicType, writable, constant);
					}
					else
					{
						return CatGenericType::unknownType;
					}
					break;
				case SpecificType::ReflectableObject:
					if (objectTypeName != "")
					{
						TypeInfo* objectType = XMLHelper::findOrCreateTypeInfo(objectTypeName, typeInfos);
						//QQQ store and load ownership semantics
						return CatGenericType(objectType, TypeOwnershipSemantics::Weak, writable, constant);
					}
					else
					{
						return CatGenericType::unknownType;
					}
					break;
				case SpecificType::None:
					return CatGenericType();
				default: 
					return CatGenericType::unknownType;
			}
		}
		else
		{
			return CatGenericType::unknownType;
		}
	}
}


void CatGenericType::writeToXML(std::ofstream& xmlFile, const char* linePrefixCharacters) const
{
	if (constant)
	{
		xmlFile << linePrefixCharacters << "<const/>\n";
	}
	if (writable)
	{
		xmlFile << linePrefixCharacters << "<writable/>\n";
	}
	if (isBasicType() || isVoidType())
	{
		xmlFile << linePrefixCharacters << "<Type>BasicType</Type>\n";		
		xmlFile << linePrefixCharacters << "<BasicType>" << toString(basicType) << "</BasicType>\n";		
	}
	else if (isReflectableObjectType())
	{
		xmlFile << linePrefixCharacters << "<Type>ObjectType</Type>\n";		
		xmlFile << linePrefixCharacters << "<ObjectTypeName>" << getObjectTypeName() << "</ObjectTypeName>\n";		
	}
	else
	{
		xmlFile << linePrefixCharacters << "<Type>None</Type>\n";		
	}
}


bool CatGenericType::isConstructible() const
{
	switch (specificType)
	{
		case SpecificType::Enum:
		case SpecificType::Basic:				return true;
		case SpecificType::ReflectableObject:	return nestedType->getAllowConstruction();
		case SpecificType::ReflectableHandle:
		case SpecificType::Pointer:
		{
			if ((ownershipSemantics == TypeOwnershipSemantics::Owned 
				|| ownershipSemantics == TypeOwnershipSemantics::Value)
				&& (isPointerToReflectableObjectType() || isPointerToHandleType()))
			{
				return pointeeType->isConstructible();
			}
			else
			{
				return true;
			}
		}
		default: assert(false); return false;
	}
}


bool CatGenericType::isCopyConstructible() const
{
	switch (specificType)
	{
		case SpecificType::Enum:
		case SpecificType::Basic:				return true;
		case SpecificType::ReflectableObject:	return nestedType->getAllowCopyConstruction();
		case SpecificType::ReflectableHandle:
		case SpecificType::Pointer:
		{
			if ((ownershipSemantics == TypeOwnershipSemantics::Owned 
				|| ownershipSemantics == TypeOwnershipSemantics::Value)
				&& (isPointerToReflectableObjectType() || isPointerToHandleType()))
			{
				return pointeeType->isCopyConstructible();
			}
			else
			{
				return true;
			}
		}
		default: assert(false); return false;
	}
}


bool CatGenericType::isMoveConstructible() const
{
	switch (specificType)
	{
		case SpecificType::Enum:
		case SpecificType::Basic:				return true;
		case SpecificType::ReflectableObject:	return nestedType->getAllowMoveConstruction();
		case SpecificType::ReflectableHandle:
		case SpecificType::Pointer:
		{
			if ((ownershipSemantics == TypeOwnershipSemantics::Owned 
				|| ownershipSemantics == TypeOwnershipSemantics::Value)
				&& (isPointerToReflectableObjectType() || isPointerToHandleType()))
			{
				return pointeeType->isMoveConstructible();
			}
			else
			{
				return true;
			}
		}
		default: assert(false); return false;
	}
}


bool CatGenericType::isDestructible() const
{
	switch (specificType)
	{
		case SpecificType::Enum:
		case SpecificType::Basic:				return true;
		case SpecificType::ReflectableObject:	return nestedType->getAllowConstruction(); //If an object is constructible it must also be destructible
		case SpecificType::ReflectableHandle:
		case SpecificType::Pointer:
		{
			if ((ownershipSemantics == TypeOwnershipSemantics::Owned 
				|| ownershipSemantics == TypeOwnershipSemantics::Value)
				&& (isPointerToReflectableObjectType() || isPointerToHandleType()))
			{
				return pointeeType->isDestructible();
			}
			else
			{
				return true;
			}
		}
		default: assert(false); return false;
	}
}


std::any CatGenericType::construct() const
{
	std::size_t typeSize = getTypeSize();
	switch (specificType)
	{
		case SpecificType::Basic:
		{
			switch (basicType)
			{
				default:
				case BasicType::Void:
				case BasicType::None:
					assert(false);
					return std::any();
				case BasicType::Float:	return 0.0f;
				case BasicType::Double:	return 0.0;
				case BasicType::Char:	return (char)0;
				case BasicType::UChar:	return (unsigned char)0;
				case BasicType::Int:	return 0;
				case BasicType::UInt:	return (unsigned int)0;
				case BasicType::Int64:	return (int64_t)0;
				case BasicType::UInt64:	return (uint64_t)0;
				case BasicType::Bool:	return false;
			} 
		} return true;
		case SpecificType::Pointer:
		case SpecificType::ReflectableHandle:
		{
			if (ownershipSemantics != TypeOwnershipSemantics::Value)
			{
				return createNullPtr();
			}
			else
			{
				unsigned char* buffer = new unsigned char[typeSize];
				if constexpr (Configuration::logJitCatObjectConstructionEvents)
				{
					std::cout << "(CatGenericType::construct Value ownership pointer) Allocated buffer of size " << std::dec << typeSize << ": " << std::hex << reinterpret_cast<uintptr_t>(buffer) << "\n";
				}
				nestedType->placementConstruct(buffer, typeSize);
				return createFromRawPointer(reinterpret_cast<uintptr_t>(buffer));
			}
		}
		case SpecificType::Enum:
		case SpecificType::ReflectableObject:
		{
			unsigned char* buffer = new unsigned char[typeSize];
			if constexpr (Configuration::logJitCatObjectConstructionEvents)
			{
				std::cout << "(CatGenericType::construct ReflectableObject) Allocated buffer of size " << std::dec << typeSize << ": " << std::hex << reinterpret_cast<uintptr_t>(buffer) << "\n";
			}
			nestedType->placementConstruct(buffer, typeSize);
			//This will copy construct whatever is contained in the raw pointer.
			std::any value = createFromRawPointer(reinterpret_cast<uintptr_t>(buffer));
			placementDestruct(buffer, typeSize);
			delete[] buffer;
			return value;
		}
		default: assert(false);
	}
	return std::any();
}


bool CatGenericType::placementConstruct(unsigned char* buffer, std::size_t bufferSize) const
{
	std::size_t typeSize = getTypeSize();
	if (typeSize > bufferSize)
	{
		assert(false);
		return false;
	}
	switch (specificType)
	{
		case SpecificType::Basic:
		{
			switch (basicType)
			{
				default:
				case BasicType::Void:
				case BasicType::None:
					return false;
				case BasicType::Float:	*reinterpret_cast<float*>(buffer) = 0.0f;		break;
				case BasicType::Double:	*reinterpret_cast<double*>(buffer) = 0.0f;		break;
				case BasicType::Char:	*reinterpret_cast<char*>(buffer) = 0;			break;
				case BasicType::UChar:	*reinterpret_cast<unsigned char*>(buffer) = 0;	break;
				case BasicType::Int:	*reinterpret_cast<int*>(buffer) = 0;			break;
				case BasicType::UInt:	*reinterpret_cast<unsigned int*>(buffer) = 0;	break;
				case BasicType::Int64:	*reinterpret_cast<int64_t*>(buffer) = 0;		break;
				case BasicType::UInt64:	*reinterpret_cast<uint64_t*>(buffer) = 0;		break;
				case BasicType::Bool:	*reinterpret_cast<bool*>(buffer) = false;		break;
			} 
		} return true;
		case SpecificType::Pointer:
		{
			if (ownershipSemantics != TypeOwnershipSemantics::Value)
			{
				memset(buffer, 0, typeSize);
			}
			else
			{
				pointeeType->placementConstruct(buffer, typeSize);
			}
			return true;
		}
		case SpecificType::ReflectableHandle:
		{
			new(buffer) ReflectableHandle();
			return true;
		}
		case SpecificType::Enum:
		case SpecificType::ReflectableObject:
		{
			nestedType->placementConstruct(buffer, bufferSize);
			return true;
		}
		default: assert(false);
	}
	return false;
}


bool CatGenericType::copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize) const
{
	std::size_t typeSize = getTypeSize();
	if (typeSize > targetBufferSize || typeSize > sourceBufferSize)
	{
		assert(false);
		return false;
	}
	switch (specificType)
	{
		case SpecificType::Enum:
		case SpecificType::Basic:
		{
			switch (basicType)
			{
				default:
				case BasicType::Void:
				case BasicType::None:
					return false;
				case BasicType::Float:	*reinterpret_cast<float*>(targetBuffer) = *reinterpret_cast<const float*>(sourceBuffer);				break;
				case BasicType::Double:	*reinterpret_cast<double*>(targetBuffer) = *reinterpret_cast<const double*>(sourceBuffer);				break;
				case BasicType::Char:	*reinterpret_cast<char*>(targetBuffer) = *reinterpret_cast<const char*>(sourceBuffer);					break;
				case BasicType::UChar:	*reinterpret_cast<unsigned char*>(targetBuffer) = *reinterpret_cast<const unsigned char*>(sourceBuffer);break;
				case BasicType::Int:	*reinterpret_cast<int*>(targetBuffer) = *reinterpret_cast<const int*>(sourceBuffer);					break;
				case BasicType::UInt:	*reinterpret_cast<unsigned int*>(targetBuffer) = *reinterpret_cast<const unsigned int*>(sourceBuffer);	break;
				case BasicType::Int64:	*reinterpret_cast<int64_t*>(targetBuffer) = *reinterpret_cast<const int64_t*>(sourceBuffer);			break;
				case BasicType::UInt64:	*reinterpret_cast<uint64_t*>(targetBuffer) = *reinterpret_cast<const uint64_t*>(sourceBuffer);			break;
				case BasicType::Bool:	*reinterpret_cast<bool*>(targetBuffer) = *reinterpret_cast<const bool*>(sourceBuffer);					break;
			} 
		} return true;
		case SpecificType::Pointer:
		{
			switch (ownershipSemantics)
			{
				case TypeOwnershipSemantics::Owned:
				{
					//Allocate heap memory for the object and copy-construct into it.
					//Then store the pointer to the heap memory.
					std::size_t objectSize = pointeeType->getTypeSize();
					unsigned char* objectMemory = new unsigned char[objectSize];
					if constexpr (Configuration::logJitCatObjectConstructionEvents)
					{
						std::cout << "(CatGenericType::copyConstruct) Allocated buffer of size " << std::dec << objectSize << ": " << std::hex << reinterpret_cast<uintptr_t>(objectMemory) << "\n";
					}
					pointeeType->copyConstruct(objectMemory, objectSize, sourceBuffer, objectSize);
					*reinterpret_cast<unsigned char**>(targetBuffer) = reinterpret_cast<unsigned char*>(objectMemory);
					break;
				}
				case TypeOwnershipSemantics::Value:
				{
					//Create a copy of the object in-place.
					pointeeType->copyConstruct(targetBuffer, targetBufferSize, sourceBuffer, sourceBufferSize);
					break;
				}
				default:
				case TypeOwnershipSemantics::Weak: //Weak and shared pointers should only exist as ReflectableHandles
				case TypeOwnershipSemantics::Shared:
					assert(false); return false;;
			}
			return true;
		}
		case SpecificType::ReflectableHandle:
		{
			switch (ownershipSemantics)
			{
				case TypeOwnershipSemantics::Owned:
				{
					const ReflectableHandle* sourceHandle = reinterpret_cast<const ReflectableHandle*>(sourceBuffer);
					unsigned char* objectMemory = nullptr;
					TypeInfo* objectType = nullptr;
					if (sourceHandle->getIsValid())
					{
						//Allocate heap memory for the object and copy-construct into it.
						//Then store the pointer to the heap memory in a ReflectableHandle.
						std::size_t objectSize = pointeeType->getTypeSize();
						objectMemory = new unsigned char[objectSize];
						objectType = sourceHandle->getObjectType();
						if constexpr (Configuration::logJitCatObjectConstructionEvents)
						{
							std::cout << "(CatGenericType::copyConstruct) Allocated buffer of size " << std::dec << objectSize << ": " << std::hex << reinterpret_cast<uintptr_t>(objectMemory) << "\n";
						}
						pointeeType->copyConstruct(objectMemory, objectSize, reinterpret_cast<unsigned char*>(sourceHandle->get()), objectSize);
					}
					new(targetBuffer) ReflectableHandle(objectMemory, objectType);
				} break;
				case TypeOwnershipSemantics::Weak:
				{
					//Simply copy construct ReflectableHandle.
					const ReflectableHandle* otherHandle = reinterpret_cast<const ReflectableHandle*>(sourceBuffer);
					if (otherHandle == nullptr)
					{
						new (targetBuffer) ReflectableHandle();
					}
					else
					{
						new (targetBuffer) ReflectableHandle(*otherHandle);
					}
				} break;
				case TypeOwnershipSemantics::Shared: //Shared pointers not yet implemented
				default:
				case TypeOwnershipSemantics::Value: assert(false); return false;
			}
			
			return true;
		}
		case SpecificType::ReflectableObject:
		{
			//Implies value ownership, copy contruct
			nestedType->copyConstruct(targetBuffer, targetBufferSize, sourceBuffer, sourceBufferSize);
			return true;
		}
		default:
		{
			assert(false);
			return false;
		}
	}
}


bool CatGenericType::moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize) const
{
	std::size_t typeSize = getTypeSize();
	if (typeSize > targetBufferSize || typeSize > sourceBufferSize)
	{
		assert(false);
		return false;
	}
	switch (specificType)
	{
		case SpecificType::Enum:
		case SpecificType::Basic:
		{
			switch (basicType)
			{
				default:
				case BasicType::Void:
				case BasicType::None:
					return false;
				case BasicType::Float:	*reinterpret_cast<float*>(targetBuffer) = *reinterpret_cast<float*>(sourceBuffer);					break;
				case BasicType::Double:	*reinterpret_cast<double*>(targetBuffer) = *reinterpret_cast<double*>(sourceBuffer);				break;
				case BasicType::Char:	*reinterpret_cast<char*>(targetBuffer) = *reinterpret_cast<char*>(sourceBuffer);					break;
				case BasicType::UChar:	*reinterpret_cast<unsigned char*>(targetBuffer) = *reinterpret_cast<unsigned char*>(sourceBuffer);	break;
				case BasicType::Int:	*reinterpret_cast<int*>(targetBuffer) = *reinterpret_cast<int*>(sourceBuffer);						break;
				case BasicType::UInt:	*reinterpret_cast<unsigned int*>(targetBuffer) = *reinterpret_cast<unsigned int*>(sourceBuffer);	break;
				case BasicType::Int64:	*reinterpret_cast<int64_t*>(targetBuffer) = *reinterpret_cast<int64_t*>(sourceBuffer);				break;
				case BasicType::UInt64:	*reinterpret_cast<uint64_t*>(targetBuffer) = *reinterpret_cast<uint64_t*>(sourceBuffer);			break;
				case BasicType::Bool:	*reinterpret_cast<bool*>(targetBuffer) = *reinterpret_cast<bool*>(sourceBuffer);					break;
			} 
		} return true;
		case SpecificType::Pointer:
		{
			switch (ownershipSemantics)
			{
				case TypeOwnershipSemantics::Owned:
				{
					//Simply copy the pointer and then set the source pointer to nullptr. Ownership is now moved.
					*reinterpret_cast<unsigned char**>(targetBuffer) = *reinterpret_cast<unsigned char**>(sourceBuffer);
					*reinterpret_cast<unsigned char**>(sourceBuffer) = nullptr;
					break;
				}
				case TypeOwnershipSemantics::Value:
				{
					//Call the move constructor of the pointee-type.
					pointeeType->moveConstruct(targetBuffer, targetBufferSize, sourceBuffer, sourceBufferSize);
					break;
				}
				default:
				case TypeOwnershipSemantics::Weak: //Weak and shared pointers should only exist as ReflectableHandles
				case TypeOwnershipSemantics::Shared:
					assert(false); return false;
			}
			return true;
		}
		case SpecificType::ReflectableHandle:
		{
			switch (ownershipSemantics)
			{
				case TypeOwnershipSemantics::Owned:
				case TypeOwnershipSemantics::Weak:
				{
					const ReflectableHandle* otherHandle = reinterpret_cast<const ReflectableHandle*>(sourceBuffer);
					//Simply copy the pointer and then set the source pointer to nullptr. Ownership is now moved.
					if (otherHandle == nullptr)
					{
						new (targetBuffer) ReflectableHandle();
					}
					else
					{
						new (targetBuffer) ReflectableHandle(*otherHandle);
						reinterpret_cast<ReflectableHandle*>(sourceBuffer)->setReflectable(nullptr, nullptr);
					}
				} break;
				case TypeOwnershipSemantics::Shared: //Shared pointers not yet implemented
				default:
				case TypeOwnershipSemantics::Value: assert(false); return false;
			}
			return true;
		}
		case SpecificType::ReflectableObject:
		{
			//Move contruct
			nestedType->moveConstruct(targetBuffer, targetBufferSize, sourceBuffer, sourceBufferSize);
			return true;
		}
		default:
		{
			assert(false);
			return false;
		}
	}
}


bool CatGenericType::placementDestruct(unsigned char* buffer, std::size_t bufferSize) const
{
	if (buffer == nullptr)
	{
		return true;
	}
	switch (specificType)
	{
		case SpecificType::Enum:
		case SpecificType::Basic:
		{
			return true;
		} break;
		case SpecificType::Pointer:
		{
			switch (ownershipSemantics)
			{
				case TypeOwnershipSemantics::Value:
				{
					pointeeType->placementDestruct(buffer, getTypeSize());
					return true;
				} break;
				case TypeOwnershipSemantics::Owned:
				{
					pointeeType->placementDestruct(*reinterpret_cast<unsigned char**>(buffer), pointeeType->getTypeSize());
					delete[] *reinterpret_cast<unsigned char**>(buffer);
					if constexpr (Configuration::logJitCatObjectConstructionEvents)
					{
						std::cout << "(CatGenericType::placementDestruc) deallocated buffer of size " << std::dec << pointeeType->getTypeSize() << ": " << std::hex << reinterpret_cast<uintptr_t>(buffer) << "\n";
					}
				} break;
				default:
				case TypeOwnershipSemantics::Weak: //Weak and shared pointers should only exist as ReflectableHandles
				case TypeOwnershipSemantics::Shared:
					assert(false); return false;;
			}

			return true;
		}
		case SpecificType::ReflectableHandle:
		{
			assert(ownershipSemantics != TypeOwnershipSemantics::Value); //ReflectableHandles cant be value owned
			assert(ownershipSemantics != TypeOwnershipSemantics::Shared); //Shared ownership not yet implemented.
			ReflectableHandle* handle = reinterpret_cast<ReflectableHandle*>(buffer);
			unsigned char* object = handle->get();
			handle->~ReflectableHandle();
			if (ownershipSemantics == TypeOwnershipSemantics::Owned)
			{
				pointeeType->placementDestruct(object, pointeeType->getTypeSize());
			}
			return true;
		}
		case SpecificType::ReflectableObject:
		{
			nestedType->placementDestruct(buffer, bufferSize);
			return true;
		}
		default: assert(false);
	}
	return true;
}


void CatGenericType::toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const
{
	assert(value.has_value());
	bufferSize = getTypeSize();
	switch (specificType)
	{
		case SpecificType::Basic:
		{
			switch (basicType)
			{
				default:
				case BasicType::Void:
				case BasicType::None:
					assert(false); break;
				case BasicType::Float:	buffer = reinterpret_cast<const unsigned char*>(std::any_cast<const float>(&value));		break;
				case BasicType::Double:	buffer = reinterpret_cast<const unsigned char*>(std::any_cast<const double>(&value));		break;
				case BasicType::Char:	buffer = reinterpret_cast<const unsigned char*>(std::any_cast<const char>(&value));			break;
				case BasicType::UChar:	buffer = reinterpret_cast<const unsigned char*>(std::any_cast<const unsigned char>(&value));break;
				case BasicType::Int:	buffer = reinterpret_cast<const unsigned char*>(std::any_cast<const int>(&value));			break;
				case BasicType::UInt:	buffer = reinterpret_cast<const unsigned char*>(std::any_cast<const unsigned int>(&value));	break;
				case BasicType::Int64:	buffer = reinterpret_cast<const unsigned char*>(std::any_cast<const int64_t>(&value));		break;
				case BasicType::UInt64:	buffer = reinterpret_cast<const unsigned char*>(std::any_cast<const uint64_t>(&value));		break;
				case BasicType::Bool:	buffer = reinterpret_cast<const unsigned char*>(std::any_cast<const bool>(&value));			break;
			} 
		} break;
		case SpecificType::Enum:
		case SpecificType::ReflectableObject:	nestedType->toBuffer(value, buffer, bufferSize); break;
		case SpecificType::Pointer:
			if (pointeeType->isReflectableObjectType())
			{
				if (ownershipSemantics == TypeOwnershipSemantics::Value)
				{
					const TypeCaster* typeCaster = getPointeeType()->getObjectType()->getTypeCaster();
					buffer = reinterpret_cast<const unsigned char*>(typeCaster->castToRawPointer(typeCaster->getAddressOfValue(const_cast<std::any&>(value))));
				}
				else
				{
					const TypeCaster* typeCaster = getPointeeType()->getObjectType()->getTypeCaster();
					buffer = reinterpret_cast<const unsigned char*>(typeCaster->castToRawPointerPointer(typeCaster->getAddressOfPointer(const_cast<std::any&>(value))));
				}
			} break;
		case SpecificType::ReflectableHandle:	
		default: assert(false); break;
	}
}


const TypeCaster* CatGenericType::getTypeCaster() const
{
	switch (specificType)
	{
		case SpecificType::Basic:
		{
			switch (basicType)
			{
				case BasicType::Bool:	return boolTypeCaster.get();
				case BasicType::Char:	return charTypeCaster.get();
				case BasicType::UChar:	return uCharTypeCaster.get();
				case BasicType::Int:	return intTypeCaster.get();
				case BasicType::UInt:	return uIntTypeCaster.get();
				case BasicType::Int64:	return int64TypeCaster.get();
				case BasicType::UInt64:	return uInt64TypeCaster.get();
				case BasicType::Float:	return floatTypeCaster.get();
				case BasicType::Double:	return doubleTypeCaster.get();
				default: assert(false);
			}
		}
		case SpecificType::Enum:
		case SpecificType::ReflectableObject:
			return nestedType->getTypeCaster();
		default: assert(false);
	}
	return nullptr;
}


uintptr_t CatGenericType::getRawPointer(const std::any& value) const
{
	assert(value.has_value());
	assert(isPointerType() || isReflectableHandleType());
	int indirectionLevels = 0;
	const CatGenericType& decayedType = removeIndirection(indirectionLevels);
	assert(decayedType.isReflectableObjectType() || decayedType.isBasicType());
	switch (indirectionLevels)
	{
		case 1:	return decayedType.getTypeCaster()->castToRawPointer(value);
		case 2:	return decayedType.getTypeCaster()->castToRawPointerPointer(value);
		default: assert(false);
	}
	return 0;
}


std::any CatGenericType::getAddressOf(std::any& value) const
{
	assert(value.has_value());
	assert(isPointerType() || isReflectableHandleType() || isBasicType() || isReflectableObjectType());
	int indirectionLevels = 0;
	const CatGenericType& decayedType = removeIndirection(indirectionLevels);
	assert(decayedType.isReflectableObjectType() || decayedType.isBasicType());
	switch (indirectionLevels)
	{
		case 0:	return decayedType.getTypeCaster()->getAddressOfValue(value);
		case 1: return decayedType.getTypeCaster()->getAddressOfPointer(value);
		default: assert(false);
	}
	return value;
}


std::any CatGenericType::getDereferencedOf(std::any& value) const
{
	assert(value.has_value());
	assert(isPointerType() || isReflectableHandleType());
	int indirectionLevels = 0;
	const CatGenericType& decayedType = removeIndirection(indirectionLevels);
	assert(decayedType.isReflectableObjectType() || decayedType.isBasicType());
	switch (indirectionLevels)
	{
		case 1:	return decayedType.getTypeCaster()->getValueOfPointer(value);
		case 2:	return decayedType.getTypeCaster()->getValueOfPointerToPointer(value);
		default: assert(false);
	}
	return std::any();
}


std::any CatGenericType::createFromRawPointer(const uintptr_t pointer) const
{
	assert(isPointerType() || isReflectableHandleType() || isReflectableObjectType());
	int indirectionLevels = 0;
	const CatGenericType& decayedType = removeIndirection(indirectionLevels);
	assert(decayedType.isReflectableObjectType() || decayedType.isBasicType());
	switch (indirectionLevels)
	{
		case 0:  
		{
			std::any ptrValue = decayedType.getTypeCaster()->castFromRawPointer(pointer);
			return decayedType.getTypeCaster()->getValueOfPointer(ptrValue);
		}
		case 1:	return decayedType.getTypeCaster()->castFromRawPointer(pointer);
		case 2:	return decayedType.getTypeCaster()->castFromRawPointerPointer(pointer);
		default: assert(false);
	}
	return nullptr;
}


std::any CatGenericType::createNullPtr() const
{
	return createFromRawPointer(0);
}


jitcat::Reflection::TypeInfo ** jitcat::CatGenericType::getTypeInfoToSet()
{
	return &nestedType;
}


const CatGenericType& CatGenericType::getWidestBasicType(const CatGenericType& left, const CatGenericType& right)
{
	assert(left.isBasicType() && right.isBasicType());
	BasicType basicType = getWidestType(left.basicType, right.basicType);
	if (basicType == left.basicType)	return left;
	else								return right;
}


CatGenericType CatGenericType::createCharType(bool isWritable, bool isConst)
{
	return CatGenericType(BasicType::Char, isWritable, isConst);
}


CatGenericType CatGenericType::createUCharType(bool isWritable, bool isConst)
{
	return CatGenericType(BasicType::UChar, isWritable, isConst);
}


CatGenericType CatGenericType::createIntType(bool isWritable, bool isConst)
{
	return CatGenericType(BasicType::Int, isWritable, isConst);
}


CatGenericType CatGenericType::createUIntType(bool isWritable, bool isConst)
{
	return CatGenericType(BasicType::UInt, isWritable, isConst);
}

CatGenericType CatGenericType::createInt64Type(bool isWritable, bool isConst)
{
	return CatGenericType(BasicType::Int64, isWritable, isConst);
}


CatGenericType jitcat::CatGenericType::createUInt64Type(bool isWritable, bool isConst)
{
	return CatGenericType(BasicType::UInt64, isWritable, isConst);
}


CatGenericType CatGenericType::createFloatType(bool isWritable, bool isConst)
{
	return CatGenericType(BasicType::Float, isWritable, isConst);
}


CatGenericType CatGenericType::createDoubleType(bool isWritable, bool isConst)
{
	return CatGenericType(BasicType::Double, isWritable, isConst);
}


CatGenericType CatGenericType::createBoolType(bool isWritable, bool isConst)
{
	return CatGenericType(BasicType::Bool, isWritable, isConst);
}


CatGenericType CatGenericType::createStringType(bool isWritable, bool isConst)
{
	return stringMemberValuePtrType.copyWithFlags(isWritable, isConst);
}


CatGenericType CatGenericType::createArrayType(const CatGenericType& arrayItemType, bool isWritable, bool isConst)
{
	return CatGenericType(&ArrayTypeInfo::createArrayTypeOf(arrayItemType), isWritable, isConst);
}


CatGenericType jitcat::CatGenericType::createBasicTypeFromName(const std::string& name, bool isWritable, bool isConst)
{
	if (Tools::equalsWhileIgnoringCase("string", name))
	{
		return stringType.copyWithFlags(isWritable, isConst);
	}
	using EnumT = typename std::underlying_type_t<BasicType>;
	for (EnumT iter = 0;  iter != static_cast<EnumT>(BasicType::Count); ++iter)
	{
		BasicType iterEnum = static_cast<BasicType>(iter);
		if (Tools::equalsWhileIgnoringCase(toString(iterEnum), name))
		{
			return CatGenericType(iterEnum, isWritable, isConst);
		}
	}
	return CatGenericType();
}


bool CatGenericType::isValidSpecificType(SpecificType type)
{
	return type != SpecificType::None && type != SpecificType::Count;
}


bool CatGenericType::isValidBasicType(BasicType type)
{
	return type != BasicType::Count && type != BasicType::None;
}


const char* CatGenericType::toString(BasicType type)
{
	switch (type)
	{
		case BasicType::Char:		return "char";
		case BasicType::UChar:		return "uchar";
		case BasicType::Int:		return "int";
		case BasicType::UInt:		return "uint";
		case BasicType::Int64:		return "int64";
		case BasicType::UInt64:		return "uint64";
		case BasicType::Float:		return "float";
		case BasicType::Double:		return "double";
		case BasicType::Bool:		return "bool";
		case BasicType::Void:		return "void";
		default:
		case BasicType::None:		return "none";
	}
}


CatGenericType::BasicType CatGenericType::toBasicType(const char* value)
{
	std::string str(value);
	std::size_t length = str.length();
	for (std::size_t i = 0; i < length; i++)
	{
		str[i] = std::tolower(str[i], Configuration::localeForStringConversions);
	}
	for (int i = 0; i < (int)BasicType::Count; i++)
	{
		if (str == toString((BasicType)i))
		{
			return (BasicType)i;
		}
	}
	return BasicType::None;
}


CatGenericType::BasicType CatGenericType::getWidestType(BasicType lType, BasicType rType)
{
	if (lType == rType)
	{
		return lType;
	}
	switch (lType)
	{
		case BasicType::Count:
		case BasicType::Void:
		case BasicType::None:
		case BasicType::UChar:
		case BasicType::Char:	
		case BasicType::Bool:	return rType;
		case BasicType::UInt:
		case BasicType::Int:
		{
			switch (rType)
			{
				default: return lType;
				case BasicType::Double:	
				case BasicType::Int64:
				case BasicType::UInt64:
				case BasicType::Float:	return rType;
			}
		}
		case BasicType::Int64:
		case BasicType::UInt64:
		{
			switch (rType)
			{
				default: return lType;
				case BasicType::Float:
				case BasicType::Double: return rType;
			}
		}
		case BasicType::Float:
			switch (rType)
			{
				default: return lType;
				case BasicType::Double:	return rType;
			}
		case BasicType::Double:	return BasicType::Double;
	}
	assert(false);
	return lType;
}


const char* CatGenericType::toString(SpecificType type)
{
	switch (type)
	{
		default:
		case SpecificType::None:				return "none";
		case SpecificType::Basic:				return "basic";
		case SpecificType::Enum:				return "enum";
		case SpecificType::Pointer:				return "pointer";
		case SpecificType::ReflectableHandle:	return "handle";
		case SpecificType::ReflectableObject:	return "object";
	}
}


CatGenericType::SpecificType CatGenericType::toSpecificType(const char * value)
{
	std::string str(value);
	std::size_t length = str.length();
	for (std::size_t i = 0; i < length; i++)
	{
		str[i] = std::tolower(str[i], Configuration::localeForStringConversions);
	}
	for (int i = 0; i < (int)SpecificType::Count; i++)
	{
		if (str == toString((SpecificType)i))
		{
			return (SpecificType)i;
		}
	}
	return SpecificType::None;
}


const CatGenericType& CatGenericType::getBasicType(BasicType type)
{
	switch (type)
	{
		case BasicType::Bool:	return boolType;
		case BasicType::Float:	return floatType;
		case BasicType::Double:	return doubleType;
		case BasicType::Char:	return charType;
		case BasicType::UChar:	return uCharType;
		case BasicType::Int:	return intType;
		case BasicType::UInt:	return uIntType;
		case BasicType::Int64:	return int64Type;
		case BasicType::UInt64:	return uInt64Type;
		case BasicType::Void:	return voidType;
		default: assert(false);	return CatGenericType::unknownType;
	}
	
}

const CatGenericType CatGenericType::voidType		= CatGenericType(BasicType::Void); 
const CatGenericType CatGenericType::charType		= CatGenericType(BasicType::Char, true);
const CatGenericType CatGenericType::uCharType		= CatGenericType(BasicType::UChar, true);
const CatGenericType CatGenericType::intType		= CatGenericType(BasicType::Int, true);
const CatGenericType CatGenericType::uIntType		= CatGenericType(BasicType::UInt, true);
const CatGenericType CatGenericType::int64Type		= CatGenericType(BasicType::Int64, true);
const CatGenericType CatGenericType::uInt64Type		= CatGenericType(BasicType::UInt64, true);
const CatGenericType CatGenericType::floatType		= CatGenericType(BasicType::Float, true);
const CatGenericType CatGenericType::doubleType		= CatGenericType(BasicType::Double, true);
const CatGenericType CatGenericType::boolType		= CatGenericType(BasicType::Bool, true);
const CatGenericType CatGenericType::stringType		= TypeTraits<Configuration::CatString>::toGenericType();
const CatGenericType CatGenericType::stringWeakPtrType = TypeTraits<Configuration::CatString>::toGenericType().toPointer(TypeOwnershipSemantics::Weak, false, false);
const CatGenericType CatGenericType::stringConstantValuePtrType = TypeTraits<Configuration::CatString>::toGenericType().toPointer(TypeOwnershipSemantics::Value, false, true);
const CatGenericType CatGenericType::stringMemberValuePtrType = TypeTraits<Configuration::CatString>::toGenericType().copyWithFlags(true, false).toPointer(TypeOwnershipSemantics::Value, true, false);
std::unique_ptr<TypeInfo, Reflection::TypeInfoDeleter> CatGenericType::nullptrTypeInfo = makeTypeInfo<TypeInfo>("nullptr", 0, std::make_unique<NullptrTypeCaster>());
CatGenericType CatGenericType::nullptrType	= CatGenericType(nullptrTypeInfo.get(), false, true).toPointer(TypeOwnershipSemantics::Value, false, true);
const CatGenericType CatGenericType::unknownType	= CatGenericType();
const std::unique_ptr<TypeCaster> CatGenericType::charTypeCaster	= std::make_unique<ObjectTypeCaster<char>>();
const std::unique_ptr<TypeCaster> CatGenericType::uCharTypeCaster	= std::make_unique<ObjectTypeCaster<unsigned char>>();
const std::unique_ptr<TypeCaster> CatGenericType::intTypeCaster		= std::make_unique<ObjectTypeCaster<int>>();
const std::unique_ptr<TypeCaster> CatGenericType::uIntTypeCaster	= std::make_unique<ObjectTypeCaster<unsigned int>>();
const std::unique_ptr<TypeCaster> CatGenericType::int64TypeCaster	= std::make_unique<ObjectTypeCaster<int64_t>>();
const std::unique_ptr<TypeCaster> CatGenericType::uInt64TypeCaster	= std::make_unique<ObjectTypeCaster<uint64_t>>();
const std::unique_ptr<TypeCaster> CatGenericType::floatTypeCaster	= std::make_unique<ObjectTypeCaster<float>>();
const std::unique_ptr<TypeCaster> CatGenericType::doubleTypeCaster	= std::make_unique<ObjectTypeCaster<double>>();
const std::unique_ptr<TypeCaster> CatGenericType::boolTypeCaster	= std::make_unique<ObjectTypeCaster<bool>>();
const std::unique_ptr<TypeCaster> CatGenericType::stringTypeCaster  = std::make_unique<ObjectTypeCaster<Configuration::CatString>>();