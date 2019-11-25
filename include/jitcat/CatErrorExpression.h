/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatTypedExpression.h"

namespace jitcat::AST
{

	class CatErrorExpression: public CatTypedExpression
	{
	public:
		CatErrorExpression(const std::string& contents, const std::string& errorMessage, const Tokenizer::Lexeme& lexeme);
		CatErrorExpression(const CatErrorExpression& other);

		virtual CatASTNode* copy() const override final;
		virtual const CatGenericType& getType() const override final;
		virtual void print() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

	private:
		std::string contents;
		std::string errorMessage;
	};

} // End namespace jitcat::AST