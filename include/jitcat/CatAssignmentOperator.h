/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatTypedExpression.h"

#include <memory>

namespace jitcat::AST
{
	class CatMemberFunctionCall;


	class CatAssignmentOperator: public CatTypedExpression
	{
	public:
		CatAssignmentOperator(CatTypedExpression* lhs, CatTypedExpression* rhs, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& operatorLexeme);
		CatAssignmentOperator(const CatAssignmentOperator& other);

		// Inherited via CatTypedExpression
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		CatTypedExpression* getLhs() const;
		CatTypedExpression* getRhs() const;

	private:
		std::unique_ptr<CatMemberFunctionCall> operatorFunction;
		std::unique_ptr<CatTypedExpression> lhs;
		std::unique_ptr<CatTypedExpression> rhs;
		Tokenizer::Lexeme operatorLexeme;
		CatGenericType type;
	};


} //End namespace jitcat::AST