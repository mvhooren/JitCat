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
#include "CatValue.h"
#include "ReflectableHandle.h"

#include <string>


//An expression that can return any type.
class ExpressionAny
{
public:
	ExpressionAny();
	ExpressionAny(const ExpressionAny& other);
	ExpressionAny(const char* expression);
	ExpressionAny(const std::string& expression);
	ExpressionAny(CatRuntimeContext* compileContext, const std::string& expression);
	~ExpressionAny();

	// Sets the expression text for this Expression
	// If compileContext == nullptr, compile needs to be called afterwards to compile the expression text
	void setExpression(const std::string& expression, CatRuntimeContext* compileContext);
	const std::string& getExpression() const;
	// Returns true if the expression is just a simple literal.
	bool isLiteral() const;

	// Returns true if the expression is constant. (It is just a literal, or a combination of operators operating on constants)
	bool isConst() const;

	// Returns true if the expression contains an error
	bool hasError() const;

	// Executes the expression and returns the value.
	const CatValue getValue(CatRuntimeContext* runtimeContext);

	void compile(CatRuntimeContext* context);

	const CatGenericType getType() const;

	//Returns the expression string (this allows serialisation without calling transSerialise on this class, saving one XML open and end tag.
	std::string& getExpressionForSerialisation();

private:
	std::string expression;
	CatGenericType valueType;
	SLRParseResult* parseResult;
	bool expressionIsLiteral;

	bool isConstant;
	CatValue cachedValue;

	//Not owned
	CatTypedExpression* expressionAST;
	ReflectableHandle errorManagerHandle;
};