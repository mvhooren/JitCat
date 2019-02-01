/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatExpression.h"
#include "jitcat/CatGenericType.h"

namespace jitcat::AST
{

	class CatTypedExpression: public CatExpression
	{
	public:
		virtual CatGenericType getType() const = 0;
		virtual bool isConst() const = 0;
		virtual bool isAssignable() const {return false;}
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) = 0;
	};

} //End namespace jitcat::AST