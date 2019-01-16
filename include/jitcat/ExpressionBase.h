/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatRuntimeContext;
class CatTypedExpression;
struct SLRParseResult;
#include "CatGenericType.h"
#include "CatType.h"
#include "CatValue.h"
#include "ReflectableHandle.h"

#include <memory>
#include <string>

//This class serves as the base class to Expression<T> and ExpressionAny, implementing shared functionality.
class ExpressionBase
{
public:
	ExpressionBase();
	ExpressionBase(const char* expression);
	ExpressionBase(const std::string& expression);
	ExpressionBase(CatRuntimeContext* compileContext, const std::string& expression);
	ExpressionBase(const ExpressionBase& other) = delete;
	virtual ~ExpressionBase();

	//Sets the expression text for this Expression
	//If compileContext == nullptr, compile needs to be called afterwards to compile the expression text
	void setExpression(const std::string& expression, CatRuntimeContext* compileContext);

	//Returns the expression.
	const std::string& getExpression() const;

	//Returns true if the expression is just a simple literal.
	bool isLiteral() const;
	//Returns true if the expression is constant. (It is just a literal, or a combination of operators operating on constants)
	bool isConst() const;

	//Returns true if the expression contains an error.
	bool hasError() const;

	//Parses the expression, checks for errors and compiles the expression to native code if the LLVM backend is enabled.
	virtual void compile(CatRuntimeContext* context) = 0;

	//Gets the type of the expression.
	const CatGenericType getType() const;

protected:
	bool parse(CatRuntimeContext* context, const CatGenericType& expectedType);

private:
	void constCollapse(CatRuntimeContext* context);
	void typeCheck(const CatGenericType& expectedType);
	void handleParseErrors(CatRuntimeContext* context);

protected:
	std::string expression;
	CatGenericType valueType;
	std::unique_ptr<SLRParseResult> parseResult;

	bool expressionIsLiteral;
	bool isConstant;

	//Not owned
	CatTypedExpression* expressionAST;
	ReflectableHandle errorManagerHandle;
};