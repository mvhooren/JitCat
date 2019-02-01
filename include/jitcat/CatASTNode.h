/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ASTNode.h"
#include "jitcat/CatASTNodeType.h"

namespace jitcat::AST
{


	class CatASTNode: public ASTNode
	{
	public:
		virtual void print() const = 0;
		virtual CatASTNodeType getNodeType() = 0;
	};


} //End namespace jitcat::AST
