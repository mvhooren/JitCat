/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

struct TypeMemberInfo;
#include "CatTypedExpression.h"
#include "RootTypeSource.h"


class CatIdentifier: public CatTypedExpression
{
public:
	//CatIdentifier();
	CatIdentifier(const std::string& name, CatRuntimeContext* context);
	virtual CatGenericType getType() const override final;
	virtual void print() const override final;
	virtual bool isConst() const override final;
	virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final;
	virtual CatASTNodeType getNodeType() override final;
	virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
	virtual CatGenericType typeCheck() override final;
	RootTypeSource getSource() const;
	const TypeMemberInfo* getMemberInfo() const;

private:
	void findIdentifier(TypeInfo* typeInfo, RootTypeSource typeSource, const std::string& lowercaseName);

public:
	std::string name;
	CatGenericType type;
	TypeMemberInfo* memberInfo;
	RootTypeSource source;
	CatRuntimeContext* compileTimeContext;
};