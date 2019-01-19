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
#include "CatGenericType.h"
#include "ReflectableHandle.h"
#include "TypeTraits.h"

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

	//Executes the expression and returns the value.
	//If isConst() == true then context may be nullptr, otherwise a context needs to be provided
	//This will execute the native-code version of the expression if the LLVM backend is enabled, otherwise it will use the interpreter.
	const T getValue(CatRuntimeContext* runtimeContext);

	//Same as getValue but will always execute the expression using the interpreter.
	//Should always return the same value as getValue. Used for testing the interpreter when the LLVM backend is enabled.
	const T getInterpretedValue(CatRuntimeContext* runtimeContext);

	virtual void compile(CatRuntimeContext* context) override final;

protected:
	virtual void handleCompiledFunction(uintptr_t functionAddress) override final;

private:
	CatType getExpectedCatType() const;
	static inline T getActualValue(const std::any& catValue);
	static inline const T getDefaultValue(CatRuntimeContext*);

private:
	const T (*getValueFunc)(CatRuntimeContext* runtimeContext);

	//If the expression is a constant, then the value is cached for performance;
	typename TypeTraits<T>::cachedType cachedValue;
};

#include "ExpressionHeaderImplementation.h"