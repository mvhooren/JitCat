/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ExpressionHelperFunctions.h"
#include "Expression.h"
#include "Tools.h"

#include <string>


bool ExpressionHelperFunctions::canBeAssigned(CatType targetType, CatType sourceType) 
{
	return targetType < CatType::Void 
		   && sourceType < CatType::Void 
		   && ((targetType == sourceType) 
		       || (targetType == CatType::Float && sourceType == CatType::Int) 
			   || (targetType == CatType::Int && sourceType == CatType::Float));
}


ExpressionBase* ExpressionHelperFunctions::createExpression(CatType type, const std::string& expressionText, CatRuntimeContext* compileContext)
{
	switch (type)
	{
		case CatType::Bool:		return new Expression<bool>(compileContext, expressionText);
		case CatType::Float:		return new Expression<float>(compileContext, expressionText);
		case CatType::Int:			return new Expression<int>(compileContext, expressionText);
		case CatType::String:		return new Expression<std::string>(compileContext, expressionText);
	}
	return nullptr;
}


ExpressionBase* ExpressionHelperFunctions::createExpression(CatType type, const std::string& expressionText)
{
	switch (type)
	{
		case CatType::Bool:	return new Expression<bool>(expressionText);
		case CatType::Float:	return new Expression<float>(expressionText);
		case CatType::Int:		return new Expression<int>(expressionText);
		case CatType::String:	return new Expression<std::string>(expressionText);
	}
	return nullptr;
}