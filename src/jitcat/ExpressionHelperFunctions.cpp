/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ExpressionHelperFunctions.h"
#include "jitcat/Expression.h"
#include "jitcat/Tools.h"

#include <string>

using namespace jitcat;


bool ExpressionHelperFunctions::canBeAssigned(const CatGenericType& targetType, const CatGenericType& sourceType) 
{
	return targetType.isBasicType()
		   && sourceType.isBasicType()
		   && ((targetType == sourceType) 
		       || (targetType.isScalarType() && sourceType.isScalarType()));
}


ExpressionBase* ExpressionHelperFunctions::createExpression(const CatGenericType& type, const std::string& expressionText, CatRuntimeContext* compileContext)
{
	if		(type.isBoolType())		return new Expression<bool>(compileContext, expressionText);
	else if (type.isFloatType())	return new Expression<float>(compileContext, expressionText);
	else if (type.isIntType())		return new Expression<int>(compileContext, expressionText);
	else if (type.isStringType())	return new Expression<std::string>(compileContext, expressionText);
	else							return nullptr;
}


ExpressionBase* ExpressionHelperFunctions::createExpression(const CatGenericType& type, const std::string& expressionText)
{
	if		(type.isBoolType())		return new Expression<bool>(expressionText);
	else if (type.isFloatType())	return new Expression<float>(expressionText);
	else if (type.isIntType())		return new Expression<int>(expressionText);
	else if (type.isStringType())	return new Expression<std::string>(expressionText);
	else							return nullptr;
}