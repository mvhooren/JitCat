/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatStatement.h"

#include <memory>
#include <vector>


namespace jitcat::AST
{
	class CatStatement;

	class CatScopeBlock: public CatStatement
	{
	public:
		CatScopeBlock(const std::vector<CatStatement*>& statementList);
		virtual ~CatScopeBlock();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

	private:
		std::vector<std::unique_ptr<CatStatement>> statements;
	};

}