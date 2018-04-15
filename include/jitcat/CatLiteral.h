/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatTypedExpression.h"


class CatLiteral: public CatTypedExpression, public CatValue
{
public:
	CatLiteral() {}
	CatLiteral(const CatValue& value): CatValue(value) {};
	CatLiteral(const std::string& value): CatValue(value) {};
	CatLiteral(float floatValue): CatValue(floatValue) {};
	CatLiteral(int intValue): CatValue(intValue) {} ;
	CatLiteral(bool boolValue): CatValue(boolValue) {};
	CatLiteral(const CatError& errorValue): CatValue(errorValue) {};

	virtual CatGenericType getType() const override final {return getValueType();} 
	virtual void print() const override final {printValue();} 
	virtual bool isConst() const override final {return true;}
	virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final {return this;}
	virtual CatASTNodeType getNodeType() override final {return CatASTNodeType::Literal;}
	virtual CatValue execute(CatRuntimeContext* runtimeContext) override final {return *this;};
	virtual CatGenericType typeCheck() override final;
};
