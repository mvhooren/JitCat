/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatArrayIndex.h"
#include "CatLog.h"
#include "MemberInfo.h"
#include "OptimizationHelper.h"


CatArrayIndex::CatArrayIndex(CatTypedExpression* base, CatTypedExpression* arrayIndex):
	base(base),
	index(arrayIndex),
	memberInfo(nullptr)
{
	if ((arrayIndex->getType() == CatType::Int
		 || arrayIndex->getType() == CatType::String)
		&& base->getType() == CatType::Object)
	{
		CatGenericType baseType = base->getType();
		if (baseType.isContainerType())
		{
			memberInfo = baseType.getContainerItemType();
		}
	}
}


void CatArrayIndex::print() const
{
	base->print();
	CatLog::log("[");
	index->print();
	CatLog::log("]");
}


CatASTNodeType CatArrayIndex::getNodeType()
{
	return CatASTNodeType::ArrayIndex;
}


CatValue CatArrayIndex::execute(CatRuntimeContext* runtimeContext)
{
	CatValue baseValue = base->execute(runtimeContext);
	CatValue indexValue = index->execute(runtimeContext);
	if (baseValue.getValueType() == CatType::Error)
	{
		return baseValue;
	}
	else if (indexValue.getValueType() == CatType::Error)
	{
		return indexValue;
	}

	if (memberInfo.isObjectType() && runtimeContext != nullptr)
	{
		MemberReferencePtr arrayReference = baseValue.getCustomTypeValue();
		if (!arrayReference.isNull() 
			&& (indexValue.getValueType() == CatType::Int || indexValue.getValueType() == CatType::String))
		{
			MemberReferencePtr itemReference;

			if ((arrayReference->getContainerType() == ContainerType::Vector 
				|| arrayReference->getContainerType() == ContainerType::StringMap)
				&& indexValue.getValueType() == CatType::Int)
			{
				int arrayIndex = indexValue.getIntValue();
				itemReference = arrayReference->getArrayItemReference(arrayIndex);
			}
			else if (arrayReference->getContainerType() == ContainerType::StringMap
					 && indexValue.getValueType() == CatType::String)
			{
				itemReference = arrayReference->getMemberReference(indexValue.getStringValue());
			}
			if (!itemReference.isNull() && itemReference->getType() == SpecificMemberType::NestedType)
			{
				return CatValue(itemReference);
			}
			else if (arrayReference->getContainerType() == ContainerType::Vector)
			{
				return CatValue(CatError(std::string("List index out of range.")));
			}
			else
			{
				return CatValue(CatError(std::string("Map item does not exist.")));
			}
		}
	}
	return CatValue(CatError(std::string("List item not found.")));
}


CatGenericType CatArrayIndex::typeCheck()
{
	CatGenericType baseType = base->typeCheck();
	CatGenericType indexType = index->typeCheck();
	if (!baseType.isValidType())
	{
		return baseType;
	}
	else if (!indexType.isValidType())
	{
		return indexType;
	}
	if (memberInfo.isObjectType())
	{
		if (!baseType.isContainerType())
		{
			return CatGenericType(Tools::append(baseType.toString(), " is not a list."));
		}
		else if (baseType.isIntType() && !indexType.isScalarType())
		{
			return CatGenericType(Tools::append(baseType.toString(), " should be indexed by a number."));
		}
		else if (baseType.isMapType() && (!indexType.isScalarType() && !indexType.isStringType()))
		{
			return CatGenericType(Tools::append(baseType.toString(), " should be indexed by a string or a number."));
		}
		else
		{
			return baseType.getContainerItemType();
		}
	}
	return CatGenericType("Invalid list or map.");
}


CatGenericType CatArrayIndex::getType() const
{
	return memberInfo;
}


bool CatArrayIndex::isConst() const
{
	return false;
}


CatTypedExpression* CatArrayIndex::constCollapse(CatRuntimeContext* compileTimeContext)
{
	OptimizationHelper::updatePointerIfChanged(base, base->constCollapse(compileTimeContext));
	OptimizationHelper::updatePointerIfChanged(index, index->constCollapse(compileTimeContext));
	return this;
}


CatTypedExpression* CatArrayIndex::getBase() const
{
	return base.get();
}


CatTypedExpression* CatArrayIndex::getIndex() const
{
	return index.get();
}
