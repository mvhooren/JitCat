/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"

#include <memory>
#include <vector>

namespace jitcat::AST
{

	class CatTypedExpression;


	class CatArgumentList: public CatASTNode
	{
	public:
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

		std::vector<std::unique_ptr<CatTypedExpression>> arguments;
	};


} //End namespace jitcat::AST