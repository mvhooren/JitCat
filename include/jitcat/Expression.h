/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatRuntimeContext;
class CatTypedExpression;
struct SLRParseResult;

#include "ExpressionBase.h"
#include "CatType.h"
#include "CatValue.h"
#include "ReflectableHandle.h"

#include <memory>
#include <string>

//An expression that can evaluate to several possible types (those types defined by CatType.h)
//This uses the JitCat compiler for parsing and executing expressions.
//Supported operators: + - / * % || && ! == != > < >= <= ( ) 
//The application can provide variables for the expression through a CatRuntimeContext
template<typename T>
class Expression: public ExpressionBase
{
public:
	Expression();
	Expression(const char* expression);
	Expression(const std::string& expression);
	Expression(CatRuntimeContext* compileContext, const std::string& expression);
	Expression(const Expression&) = delete;
	virtual ~Expression();

	//Sets the expression text for this Expression
	//If compileContext == nullptr, compile needs to be called afterwards to compile the expression text
	virtual void setExpression(const std::string& expression, CatRuntimeContext* compileContext) override final;
	virtual const std::string& getExpression() const override final;

	//Returns true if the expression is just a simple literal.
	virtual bool isLiteral() const override final;

	//Returns true if the expression is constant. (It is just a literal, or a combination of operators operating on constants)
	virtual bool isConst() const override final;

	//Returns true if the expression contains an error
	virtual bool hasError() const override final;

	//Executes the expression and returns the value.
	//If isConst() == true then context may be nullptr, otherwise a context needs to be provided
	const T getValue(CatRuntimeContext* runtimeContext);

	virtual void compile(CatRuntimeContext* context) override final;
	SLRParseResult* parse(CatRuntimeContext* context);//TTT test parsing without directly printing errors.

	virtual CatType getType() const override final;

	//Returns the expression string (this allows serialisation without calling transSerialise on this class, saving one XML open and end tag.
	virtual std::string& getExpressionForSerialisation() override final;

private:
	CatType getCatType() const;
	inline T getActualValue(const CatValue& catValue);

private:
	std::string expression;
	std::unique_ptr<SLRParseResult> parseResult;
	bool expressionIsLiteral;

	bool isConstant;
	//If the expression is a constant, then the value is cached for performance;
	T cachedValue;
	//Not owned
	CatTypedExpression* expressionAST;
	ReflectableHandle errorManagerHandle;
};

#include "ExpressionHeaderImplementation.h"