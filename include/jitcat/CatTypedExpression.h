/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatExpression.h"
#include "CatGenericType.h"


class CatTypedExpression: public CatExpression
{
public:
	virtual CatGenericType getType() const = 0;
	virtual bool isConst() const = 0;
	virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) = 0;
};