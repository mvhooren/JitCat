/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ExpressionHelperFunctions.h"
#include "jitcat/Configuration.h"
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
	if		(type.isBoolType())			return new Expression<bool>(compileContext, expressionText);
	else if (type.isFloatType())		return new Expression<float>(compileContext, expressionText);
	else if (type.isDoubleType())		return new Expression<double>(compileContext, expressionText);
	else if (type.isIntType())			return new Expression<int>(compileContext, expressionText);
	else if (type.isStringValueType())	return new Expression<Configuration::CatString>(compileContext, expressionText);
	else if (type.isStringPtrType())	return new Expression<Configuration::CatString*>(compileContext, expressionText);
	else								return nullptr;
}


ExpressionBase* ExpressionHelperFunctions::createExpression(const CatGenericType& type, const std::string& expressionText)
{
	if		(type.isBoolType())			return new Expression<bool>(expressionText);
	else if (type.isDoubleType())		return new Expression<double>(expressionText);
	else if (type.isFloatType())		return new Expression<float>(expressionText);
	else if (type.isIntType())			return new Expression<int>(expressionText);
	else if (type.isStringValueType())	return new Expression<Configuration::CatString>(expressionText);
	else if (type.isStringPtrType())	return new Expression<Configuration::CatString*>(expressionText);
	else								return nullptr;
}


std::string jitcat::ExpressionHelperFunctions::getUniqueExpressionFunctionName(const std::string& expression, CatRuntimeContext* context, bool isAssignExpression)
{
	std::hash<std::string> stringHash;
	std::size_t expressionHash = stringHash(expression);
	std::size_t contextHash = context->getContextHash();
	if (isAssignExpression)
		return Tools::append("expressionAssign_", Tools::toHexBytes(expressionHash), "_", Tools::toHexBytes(contextHash));
	else 
		return Tools::append("expression_", Tools::toHexBytes(expressionHash), "_", Tools::toHexBytes(contextHash));
}

