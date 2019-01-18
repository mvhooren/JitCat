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

#include <any>
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
	// To get the actual value contained in std::any, cast it using std::any_cast based this expression's type (getType()) .
	const std::any getValue(CatRuntimeContext* runtimeContext);

	virtual void compile(CatRuntimeContext* context) override final;

protected:
	virtual void handleCompiledFunction(uintptr_t functionAddress) override final;

private:
	std::any cachedValue;
	uintptr_t nativeFunctionAddress;
};