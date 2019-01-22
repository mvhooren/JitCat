#pragma once

#include "CatTypedExpression.h"

#include <memory>


class CatAssignmentOperator: public CatTypedExpression
{
public:
	CatAssignmentOperator(CatTypedExpression* lhs, CatTypedExpression* rhs);
	// Inherited via CatTypedExpression
	virtual void print() const override;
	virtual CatASTNodeType getNodeType() override;
	virtual std::any execute(CatRuntimeContext* runtimeContext) override;
	virtual CatGenericType typeCheck() override;
	virtual CatGenericType getType() const override;
	virtual bool isConst() const override;
	virtual CatTypedExpression * constCollapse(CatRuntimeContext* compileTimeContext) override;

private:
	std::unique_ptr<CatTypedExpression> lhs;
	std::unique_ptr<CatTypedExpression> rhs;
};