/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatRuntimeContext;
#include "CatASTNode.h"
#include "CatValue.h"


class CatExpression: public CatASTNode
{
public:
	virtual CatValue execute(CatRuntimeContext* runtimeContext) = 0;
	virtual CatGenericType typeCheck() = 0;
};