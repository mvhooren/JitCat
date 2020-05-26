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
		CatScopeRoot(const CatScopeRoot& other);

		// Inherited via CatTypedExpression
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual CatStatement* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		CatScopeID getScopeId() const;

	private:
		CatScopeID scopeId;
		CatGenericType type;
	};

} //End namespace jitcat::AST