/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatRuntimeContext;
struct SLRParseResult;
#include "CatGenericType.h"
#include "ExpressionBase.h"

#include <string>


//An expression that can return any type.
class ExpressionAny: public ExpressionBase
{
public:
	ExpressionAny();
	ExpressionAny(const char* expression);
	ExpressionAny(const std::string& expression);
	ExpressionAny(CatRuntimeContext* compileContext, const std::string& expression);
	ExpressionAny(const ExpressionAny& other) = delete;

	// Executes the expression and returns the value.
	const CatValue getValue(CatRuntimeContext* runtimeContext);
	virtual void compile(CatRuntimeContext* context) override final;

private:
	CatValue cachedValue;
};