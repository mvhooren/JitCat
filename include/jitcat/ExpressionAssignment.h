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


	//This class is similar to Expression and ExpressionAny but in this case the expressions should return a assignable type.
	//A value is then assigned to the result of the expression (using assignValue or assignInterpretedValue).
	template<typename ExpressionT>
	class ExpressionAssignment: public ExpressionBase
	{
	public:
		ExpressionAssignment();
		ExpressionAssignment(const char* expression);
		ExpressionAssignment(const std::string& expression);
		ExpressionAssignment(CatRuntimeContext* compileContext, const std::string& expression);
		ExpressionAssignment(const ExpressionAssignment&) = delete;
		virtual ~ExpressionAssignment();

		//Executes the expression and assigns the value parameter to the result of the expression.
		//This will execute the native-code version of the expression if the LLVM backend is enabled, otherwise it will use the interpreter.
		//Returns true if assignment was successful
		bool assignValue(CatRuntimeContext* runtimeContext, typename TypeTraits<ExpressionT>::functionParameterType value);

		//Same as assignValue but will always execute the expression using the interpreter.
		//Should always behave the same as assignValue. Used for testing the interpreter when the LLVM backend is enabled.
		//Returns true if assignment was successful
		bool assignInterpretedValue(CatRuntimeContext* runtimeContext, typename TypeTraits<ExpressionT>::functionParameterType value);

		//Parses the expression, checks for errors and compiles the expression to native code if the LLVM backend is enabled.
		virtual void compile(CatRuntimeContext* context) override final;

	protected:
		virtual void handleCompiledFunction(uintptr_t functionAddress) override final;

	private:
		CatGenericType getExpectedCatType() const;

	private:
		void (*assignValueFunc)(CatRuntimeContext* runtimeContext, typename TypeTraits<ExpressionT>::functionParameterType value);
		static void defaultAssignFunction(CatRuntimeContext* runtimeContext, typename TypeTraits<ExpressionT>::functionParameterType value) {}
	};

}
#include "jitcat/ExpressionAssignmentHeaderImplementation.h"
