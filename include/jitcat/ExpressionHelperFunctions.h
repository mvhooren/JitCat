/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatRuntimeContext;
class CatVariableInfo;
class ExpressionBase;
class MenuObjectInstanceLocals;
#include "CatType.h"

#include <map>
#include <string>
#include <vector>


class ExpressionHelperFunctions
{
	ExpressionHelperFunctions();

public:
	static bool canBeAssigned(CatType targetType, CatType sourceType);
	static ExpressionBase* createExpression(CatType type, const std::string& expressionText, CatRuntimeContext* compileContext);
	static ExpressionBase* createExpression(CatType type, const std::string& expressionText);
};


