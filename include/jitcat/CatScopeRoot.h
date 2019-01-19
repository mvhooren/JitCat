/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatRuntimeContext;
class TypeInfo;
#include "CatTypedExpression.h"
#include "RootTypeSource.h"


class CatScopeRoot: public CatTypedExpression
{
public:
	CatScopeRoot(RootTypeSource source, CatRuntimeContext* context);
	// Inherited via CatTypedExpression
	virtual void print() const override;
	virtual CatASTNodeType getNodeType() override;
	virtual std::any execute(CatRuntimeContext* runtimeContext) override;
	virtual CatGenericType typeCheck() override;
	virtual CatGenericType getType() const override;
	virtual bool isConst() const override;
	virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override;

	RootTypeSource getSource() const;

private:
	RootTypeSource source;
	CatGenericType type;
};