/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatTypedExpression.h"

#include <memory>


namespace jitcat::AST
{
	class CatTypedExpression;

	class CatReturnStatement: public CatTypedExpression
	{
	public:
		CatReturnStatement(const Tokenizer::Lexeme& lexeme, CatTypedExpression* returnExpression = nullptr);
		CatReturnStatement(const CatReturnStatement& other);
		virtual ~CatReturnStatement();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		virtual std::any execute(CatRuntimeContext * runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual std::optional<bool> checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected) const override final;

	private:
		std::unique_ptr<CatTypedExpression> returnExpression;
	};

}