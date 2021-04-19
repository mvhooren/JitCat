/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"

#include <map>
#include <string>
#include <vector>

namespace jitcat
{
	class CatRuntimeContext;
	class ExpressionBase;


	class ExpressionHelperFunctions
	{
		ExpressionHelperFunctions();

	public:
		static bool canBeAssigned(const CatGenericType& targetType, const CatGenericType& sourceType);
		static ExpressionBase* createExpression(const CatGenericType& type, const std::string& expressionText, CatRuntimeContext* compileContext);
		static ExpressionBase* createExpression(const CatGenericType& type, const std::string& expressionText);
		static std::string getUniqueExpressionFunctionName(const std::string& expression, CatRuntimeContext* context, bool isAssignExpression);
	};


} //End namespace jitcat