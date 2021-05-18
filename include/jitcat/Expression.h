/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ExpressionBase.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/TypeTraits.h"

#include <memory>
#include <string>

namespace jitcat
{
	class CatRuntimeContext;
	namespace AST
	{
		class CatTypedExpression;
	}
	struct SLRParseResult;


	//An expression that can evaluate to several possible types (those types defined by CatGenericType.h)
	//This uses the JitCat compiler for parsing and executing expressions.
	//The application can provide variables for the expression through a CatRuntimeContext
	template<typename ExpressionResultT>
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
		const ExpressionResultT getValue(CatRuntimeContext* runtimeContext);

		//Same as getValue but will always execute the expression using the interpreter.
		//Should always return the same value as getValue. Used for testing the interpreter when the LLVM backend is enabled.
		const ExpressionResultT getInterpretedValue(CatRuntimeContext* runtimeContext);

		//Parses the expression, checks for errors and compiles the expression to native code if the LLVM backend is enabled.
		virtual void compile(CatRuntimeContext* context) override final;

	protected:
		virtual void handleCompiledFunction(uintptr_t functionAddress) override final;
		virtual void resetCompiledFunctionToDefault() override final;

		const ExpressionResultT (Expression<ExpressionResultT>::*getValuePtr)(CatRuntimeContext* runtimeContext);
		const ExpressionResultT getExecuteInterpretedValue(CatRuntimeContext* runtimeContext);
		const ExpressionResultT getCachedValue(CatRuntimeContext* runtimeContext);
		const ExpressionResultT getDefaultValue(CatRuntimeContext* runtimeContext);

	private:
		CatGenericType getExpectedCatType() const;
		static inline ExpressionResultT getActualValue(const std::any& catValue);

	private:
		//If the expression is a constant, then the value is cached for performance;
		typename TypeTraits<ExpressionResultT>::cachedType cachedValue;
	};


} //End namespace jitcat

#include "jitcat/ExpressionHeaderImplementation.h"


