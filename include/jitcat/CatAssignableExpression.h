/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "AssignableType.h"
#include "CatTypedExpression.h"


class CatAssignableExpression: public CatTypedExpression
{
public:
	virtual bool isAssignable() const override final {return true;}
	virtual std::any executeAssignable(CatRuntimeContext* runtimeContext, AssignableType& assignableType) = 0;
};