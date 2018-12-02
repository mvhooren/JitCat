/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatTypedExpression.h"
#include "CatGenericType.h"

#include <memory>


class CatArrayIndex: public CatTypedExpression
{
public:
	CatArrayIndex(CatTypedExpression* base, CatTypedExpression* arrayIndex);
	CatArrayIndex(const CatArrayIndex&) = delete;
	virtual void print() const override final;
	virtual CatASTNodeType getNodeType() override final;
	virtual CatValue execute(CatRuntimeContext* runtimeContext) override final;
	virtual CatGenericType typeCheck() override final;

	virtual CatGenericType getType() const override final;
	virtual bool isConst() const override final;
	virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;

	CatTypedExpression* getBase() const;
	CatTypedExpression* getIndex() const;

private:
	std::unique_ptr<CatTypedExpression> base;
	std::unique_ptr<CatTypedExpression> index;
	CatGenericType memberInfo;
};