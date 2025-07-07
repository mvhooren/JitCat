/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/ExpressionBase.h"

#include <any>
#include <string>

namespace jitcat
{
	class CatRuntimeContext;
	struct SLRParseResult;


	//An expression that can return any type (among supported types).
	class ExpressionAny: public ExpressionBase
	{
	public:
		ExpressionAny();
		ExpressionAny(const char* expression);
		ExpressionAny(const std::string& expression);
		ExpressionAny(CatRuntimeContext* compileContext, const std::string& expression);
		ExpressionAny(const ExpressionAny& other) = delete;

		//Executes the expression and returns the value.
		//To get the actual value contained in std::any, cast it using std::any_cast based on this expression's type (getType()) .
		//Objects and stl containers are always returned as pointers.
		std::any getValue(CatRuntimeContext* runtimeContext);

		//Same as getValue but will always execute the expression using the interpreter.
		//Should always return the same value as getValue. Used for testing the interpreter when the LLVM backend is enabled.
		//This function will only work if either native code compilation is disabled or DiscardASTAfterNativeCodeCompilation is set to false (see JitCat.h).
		std::any getInterpretedValue(CatRuntimeContext* runtimeContext);

		virtual void compile(CatRuntimeContext* context) override final;

	protected:
		virtual void handleCompiledFunction(uintptr_t functionAddress) override final;
		virtual void resetCompiledFunctionToDefault() override final;

	private:
		const std::any (ExpressionAny::*getValuePtr)(CatRuntimeContext* runtimeContext);

		const std::any getExecuteVoidValue(CatRuntimeContext* runtimeContext);
		const std::any getExecuteBoolValue(CatRuntimeContext* runtimeContext);
		const std::any getExecuteIntValue(CatRuntimeContext* runtimeContext);
		const std::any getExecuteFloatValue(CatRuntimeContext* runtimeContext);
		const std::any getExecuteDoubleValue(CatRuntimeContext* runtimeContext);
		const std::any getExecuteReflectablePtrValue(CatRuntimeContext* runtimeContext);
		const std::any getExecutePtrPtrValue(CatRuntimeContext* runtimeContext);

		const std::any getExecuteInterpretedValue(CatRuntimeContext* runtimeContext);
		const std::any getCachedValue(CatRuntimeContext* runtimeContext);
		const std::any getDefaultValue(CatRuntimeContext* runtimeContext);

	private:
		std::any cachedValue;
		uintptr_t nativeFunctionAddress;
	};

}