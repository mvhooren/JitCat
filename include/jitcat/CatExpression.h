/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class CatRuntimeContext;
}
#include "jitcat/CatASTNode.h"
#include "jitcat/CatGenericType.h"

#include <any>

namespace jitcat::AST
{

	class CatExpression: public CatASTNode
	{
	public:
		virtual std::any execute(CatRuntimeContext* runtimeContext) = 0;
		virtual CatGenericType typeCheck() = 0;
	};


} //End namespace jitcat::AST