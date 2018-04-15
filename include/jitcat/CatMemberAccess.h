/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

struct TypeMemberInfo;
#include "CatTypedExpression.h"

#include <memory>


class CatMemberAccess: public CatTypedExpression
{
public:
	CatMemberAccess(CatTypedExpression* base, const std::string& memberName);
	CatMemberAccess(const CatMemberAccess&) = delete;
	virtual void print() const override final;
	virtual CatASTNodeType getNodeType() override final;
	virtual CatValue execute(CatRuntimeContext* runtimeContext) override final;
	virtual CatGenericType typeCheck() override final;
	virtual CatGenericType getType() const override final;
	virtual bool isConst() const override final;
	virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;

private:
	std::unique_ptr<CatTypedExpression> base;
	TypeMemberInfo* memberInfo;
	CatGenericType type;
	const std::string memberName;
};
