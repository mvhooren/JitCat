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

	class CatPrefixOperator: public CatTypedExpression
	{
	public:
		enum class Operator
		{
			Not,
			Minus,
			Count
		};

		CatPrefixOperator(const Tokenizer::Lexeme& lexeme, Operator oper, CatTypedExpression* rhs);
		CatPrefixOperator(const CatPrefixOperator& other);

		static const char* conversionTable[(unsigned int)Operator::Count];

		virtual CatASTNode* copy() const override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual CatASTNodeType getNodeType() const override final {return CatASTNodeType::PrefixOperator;}

		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		virtual void print() const override final;

		const CatTypedExpression* getRHS() const;
		CatPrefixOperator::Operator getOperator() const;

	private:
		inline std::any calculateExpression(CatRuntimeContext* runtimeContext);
		CatGenericType resultType;
		Operator oper;
		std::unique_ptr<CatTypedExpression> rhs;
	};


} // End namespace jitcat::AST