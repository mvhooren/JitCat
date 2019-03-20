/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"
#include "jitcat/CatError.h"

#include <optional>



namespace jitcat::AST
{
	class CatDefinition: public CatASTNode
	{
	public:
		CatDefinition() {};
		virtual ~CatDefinition() {};
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) = 0;
	};

};
