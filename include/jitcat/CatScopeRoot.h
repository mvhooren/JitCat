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
#include "jitcat/CatTypedExpression.h"
#include "jitcat/CatScopeID.h"

namespace jitcat::AST
{

	class CatScopeRoot: public CatTypedExpression
	{
	public:
		CatScopeRoot(CatScopeID scopeId, const Tokenizer::Lexeme& lexeme);
		// Inherited via CatTypedExpression
		virtual void print() const override;
		virtual CatASTNodeType getNodeType() override;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual CatGenericType getType() const override;
		virtual bool isConst() const override;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override;

		CatScopeID getScopeId() const;

	private:
		CatScopeID scopeId;
		CatGenericType type;
	};

} //End namespace jitcat::AST