/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "OptimizationHelper.h"
#include "CatTypedExpression.h"


void OptimizationHelper::updatePointerIfChanged(std::unique_ptr<CatTypedExpression>& uPtr, CatTypedExpression* expression)
{
	if (uPtr.get() != expression)
	{
		uPtr.reset(expression);
	}
}
