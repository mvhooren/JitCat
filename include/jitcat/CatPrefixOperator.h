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
		CatPrefixOperator(): oper(Operator::Not) {};
		CatPrefixOperator(const CatPrefixOperator&) = delete;

		enum class Operator
		{
			Not,
			Minus,
			Count
		};
		Operator oper;
		std::unique_ptr<CatTypedExpression> rhs;

		static const char* conversionTable[(unsigned int)Operator::Count];

		virtual CatGenericType getType() const override final;
		virtual bool isConst() const override final;
		virtual CatASTNodeType getNodeType() override final {return CatASTNodeType::PrefixOperator;}

		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual CatGenericType typeCheck() override final;

		virtual void print() const override final;

	private:
		inline std::any calculateExpression(CatRuntimeContext* runtimeContext);
	};


} // End namespace jitcat::AST