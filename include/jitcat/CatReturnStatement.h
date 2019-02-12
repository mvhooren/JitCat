#pragma once

#include "jitcat/CatTypedExpression.h"

#include <memory>


namespace jitcat::AST
{
	class CatTypedExpression;

	class CatReturnStatement: public CatTypedExpression
	{
	public:
		CatReturnStatement(CatTypedExpression* returnExpression = nullptr);
		virtual ~CatReturnStatement();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

		virtual std::any execute(CatRuntimeContext * runtimeContext) override final;
		virtual CatGenericType typeCheck() override final;
		virtual CatGenericType getType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;

	private:
		std::unique_ptr<CatTypedExpression> returnExpression;

	};

}