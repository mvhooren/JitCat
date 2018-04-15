/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


class CatRuntimeContext;
class ExpressionAny;
#include "CatValue.h"

#include <string>


//Tools for working with expressions
class ExpressionTools
{
public:
	//Beware that it is much much more efficient to use Expression<T> objects.
	static CatValue executeExpression(const std::string& expression, CatRuntimeContext* context);
	static bool assignIfPossible(CatValue& target, const CatValue& source);
	static void checkAssignment(ExpressionAny& target, ExpressionAny& source, CatRuntimeContext* context, void* errorSource);
};