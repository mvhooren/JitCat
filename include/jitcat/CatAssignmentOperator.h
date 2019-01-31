/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatTypedExpression.h"

#include <memory>


class CatAssignmentOperator: public CatTypedExpression
{
public:
	CatAssignmentOperator(CatTypedExpression* lhs, CatTypedExpression* rhs);
	// Inherited via CatTypedExpression
	virtual void print() const override final;
	virtual CatASTNodeType getNodeType() override final;
	virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
	virtual CatGenericType typeCheck() override final;
	virtual CatGenericType getType() const override final;
	virtual bool isConst() const override final;
	virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;

	CatTypedExpression* getLhs() const;
	CatTypedExpression* getRhs() const;

private:
	std::unique_ptr<CatTypedExpression> lhs;
	std::unique_ptr<CatTypedExpression> rhs;
};