/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatRuntimeContext;

#include "CatType.h"
#include <string>

//This class serves as the base class to Expression<T>.
//It allows the storage and manipulation of any expression by hiding the T through inheritance.
class ExpressionBase
{
public:
	//Sets the expression text for this Expression
	//compile needs to be called afterwards to compile the expression text
	virtual void setExpression(const std::string& expression, CatRuntimeContext* compileContext) = 0;
	virtual const std::string& getExpression() const = 0;
	//Returns true if the expression is just a simple literal.
	virtual bool isLiteral() const = 0;
	//Returns true if the expression is constant. (It is just a literal, or a combination of operators operating on constants)
	virtual bool isConst() const = 0;

	//Returns true if the expression contains an error
	virtual bool hasError() const = 0;

	virtual void compile(CatRuntimeContext* context) = 0;

	virtual CatType getType() const = 0;

	//Returns the expression string (this allows serialisation without calling transSerialise on this class, saving one XML open and end tag.
	virtual std::string& getExpressionForSerialisation() = 0;
};