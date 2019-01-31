/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatArrayIndex.h"
#include "CatLog.h"
#include "MemberInfo.h"
#include "ASTHelper.h"
#include "TypeInfo.h"


CatArrayIndex::CatArrayIndex(CatTypedExpression* base, CatTypedExpression* arrayIndex):
	array(base),
	index(arrayIndex)
{
	arrayType = base->getType();
	indexType = index->getType();
	if ((indexType.isIntType() || indexType.isStringType())
		 && arrayType.isContainerType())
	{
		containerItemType = arrayType.getContainerItemType();
	}
}


void CatArrayIndex::print() const
{
	array->print();
	CatLog::log("[");
	index->print();
	CatLog::log("]");
}


CatASTNodeType CatArrayIndex::getNodeType()
{
	return CatASTNodeType::ArrayIndex;
}


std::any CatArrayIndex::execute(CatRuntimeContext* runtimeContext)
{
	std::any arrayValue = array->execute(runtimeContext);
	std::any indexValue = index->execute(runtimeContext);
	if (arrayType.isMapType())
	{
		if (indexType.isIntType())
		{
			return containerItemType.getObjectType()->getTypeCaster()->getItemOfStringIndexedMapOf(arrayValue, std::any_cast<int>(indexValue));
		}
		else if (indexType.isStringType())
		{
			return containerItemType.getObjectType()->getTypeCaster()->getItemOfStringIndexedMapOf(arrayValue, std::any_cast<std::string>(indexValue));
		}
	}
	else if (arrayType.isVectorType())
	{
		return containerItemType.getObjectType()->getTypeCaster()->getItemOfVectorOf(arrayValue, std::any_cast<int>(indexValue));
	}
	return containerItemType.createDefault();
}


CatGenericType CatArrayIndex::typeCheck()
{
	CatGenericType baseType = array->typeCheck();
	CatGenericType indexType = index->typeCheck();
	if (!baseType.isValidType())
	{
		return baseType;
	}
	else if (!indexType.isValidType())
	{
		return indexType;
	}
	if (containerItemType.isObjectType())
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
	return containerItemType;
}


bool CatArrayIndex::isConst() const
{
	return false;
}


CatTypedExpression* CatArrayIndex::constCollapse(CatRuntimeContext* compileTimeContext)
{
	ASTHelper::updatePointerIfChanged(array, array->constCollapse(compileTimeContext));
	ASTHelper::updatePointerIfChanged(index, index->constCollapse(compileTimeContext));
	return this;
}


CatTypedExpression* CatArrayIndex::getBase() const
{
	return array.get();
}


CatTypedExpression* CatArrayIndex::getIndex() const
{
	return index.get();
}
