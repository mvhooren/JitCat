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

	namespace Reflection
	{
		struct MemberFunctionInfo;
	}

	//An expression that can return any type (among supported types).
	class ExpressionAssignAny : public ExpressionBase
	{
	public:
		ExpressionAssignAny();
		ExpressionAssignAny(const char* expression);
		ExpressionAssignAny(const std::string& expression);
		ExpressionAssignAny(CatRuntimeContext* compileContext, const std::string& expression);
		ExpressionAssignAny(const ExpressionAssignAny& other) = delete;


		//Executes the expression and assigns the value parameter to the result of the expression.
		//This will execute the native-code version of the expression if the LLVM backend is enabled, otherwise it will use the interpreter.
		//Returns true if assignment was successful
		template<typename T>
		bool assignValue(CatRuntimeContext* runtimeContext, T* value, const CatGenericType& valueType)
		{
			return assignUncastedPointer(runtimeContext, reinterpret_cast<uintptr_t>(value), valueType);
		}

		//Executes the expression and assigns the value parameter to the result of the expression.
		//This will execute the native-code version of the expression if the LLVM backend is enabled, otherwise it will use the interpreter.
		//Returns true if assignment was successful
		bool assignValue(CatRuntimeContext* runtimeContext, std::any value, const CatGenericType& valueType);


		template<typename T>
		bool assignInterpretedValue(CatRuntimeContext* runtimeContext, T* value, const CatGenericType& valueType)
		{
			return assignInterpretedUncastedPointer(runtimeContext, reinterpret_cast<uintptr_t>(value), valueType);
		}

		//Same as assignValue but will always execute the expression using the interpreter.
		//Should always behave the same as assignValue. Used for testing the interpreter when the LLVM backend is enabled.
		//Returns true if assignment was successful
		bool assignInterpretedValue(CatRuntimeContext* runtimeContext, std::any value, const CatGenericType& valueType);


		virtual void compile(CatRuntimeContext* context) override final;

	protected:
		virtual void handleCompiledFunction(uintptr_t functionAddress) override final;
		virtual void resetCompiledFunctionToDefault() override final;

	private:
		bool assignUncastedPointer(CatRuntimeContext* runtimeContext, uintptr_t pointerValue, const CatGenericType& valueType);
		bool assignInterpretedUncastedPointer(CatRuntimeContext* runtimeContext, uintptr_t pointerValue, const CatGenericType& valueType);

	private:
		uintptr_t nativeFunctionAddress;
		Reflection::MemberFunctionInfo* assignmentOperatorFunction;
	};

}