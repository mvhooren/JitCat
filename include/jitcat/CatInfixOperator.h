/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatTypedExpression.h"

#include <memory>


class CatInfixOperator: public CatTypedExpression
{
public:
	CatInfixOperator(): oper(CatInfixOperatorType::Plus) {}
	CatInfixOperator(const CatInfixOperator&) = delete;

	std::unique_ptr<CatTypedExpression> lhs;
	CatInfixOperatorType oper;
	std::unique_ptr<CatTypedExpression> rhs;

	virtual CatGenericType getType() const override final;
	virtual bool isConst() const override final;
	virtual CatASTNodeType getNodeType() override final {return CatASTNodeType::InfixOperator;}

	virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;
	virtual std::any execute(CatRuntimeContext* runtimeContext) override final;

	virtual CatGenericType typeCheck() override final;

	virtual void print() const override final;

private:
	inline std::any calculateExpression(CatRuntimeContext* runtimeContext);

	template<typename T, typename U, typename V>
	inline std::any calculateScalarExpression(const T& lValue, const U& rValue);
	
	template<typename T, typename U>
	inline std::any calculateStringExpression(const T& lValue, const U& rValue);

	inline std::any calculateStringExpression(const std::string& lValue, const std::string& rValue);

	inline std::any calculateBooleanExpression(bool lValue, bool rValue);
};


#include "CatInfixOperatorHeaderImplementation.h"