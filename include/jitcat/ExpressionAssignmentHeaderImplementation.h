/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "ASTHelper.h"
#include "CatAssignableExpression.h"
#include "CatTypedExpression.h"
#include "Configuration.h"


template<typename ExpressionT>
inline ExpressionAssignment<ExpressionT>::ExpressionAssignment():
	ExpressionBase(true),
	assignValueFunc(&defaultAssignFunction)
{
}


template<typename ExpressionT>
inline ExpressionAssignment<ExpressionT>::ExpressionAssignment(const char* expression):
	ExpressionBase(expression, true),
	assignValueFunc(&defaultAssignFunction)
{
}


template<typename ExpressionT>
inline ExpressionAssignment<ExpressionT>::ExpressionAssignment(const std::string& expression):
	ExpressionBase(expression, true),
	assignValueFunc(&defaultAssignFunction)
{
}


template<typename ExpressionT>
inline ExpressionAssignment<ExpressionT>::ExpressionAssignment(CatRuntimeContext* compileContext, const std::string& expression):
	ExpressionBase(compileContext, expression, true),
	assignValueFunc(&defaultAssignFunction)
{
	compile(compileContext);
}


template<typename ExpressionT>
inline ExpressionAssignment<ExpressionT>::~ExpressionAssignment()
{
}


template<typename ExpressionT>
inline bool ExpressionAssignment<ExpressionT>::assignValue(CatRuntimeContext* runtimeContext, typename TypeTraits<ExpressionT>::functionParameterType value)
{
	if constexpr (Configuration::enableLLVM)
	{
		assignValueFunc(runtimeContext, value);
		return !hasError();
	}
	else
	{
		return assignInterpretedValue(runtimeContext, value);
	}
	return false;
}


template<typename ExpressionT>
inline bool ExpressionAssignment<ExpressionT>::assignInterpretedValue(CatRuntimeContext* runtimeContext, typename TypeTraits<ExpressionT>::functionParameterType value)
{
	if (expressionAST != nullptr && expressionAST->isAssignable())
	{
		CatAssignableExpression* assignable = static_cast<CatAssignableExpression*>(expressionAST);
		AssignableType assignableType = AssignableType::None;
		std::any target = assignable->executeAssignable(runtimeContext, assignableType);
		std::any anyValue = TypeTraits<ExpressionT>::getCatValue(value);
		ASTHelper::doAssignment(target, anyValue, getExpectedCatType(), assignableType);
		return true;
	}
	return false;
}


template<typename ExpressionT>
inline void ExpressionAssignment<ExpressionT>::compile(CatRuntimeContext* context)
{
	if (!parse(context, TypeTraits<ExpressionT>::toGenericType()))
	{
		assignValueFunc = &defaultAssignFunction;
	}
}


template<typename ExpressionT>
inline void ExpressionAssignment<ExpressionT>::handleCompiledFunction(uintptr_t functionAddress)
{
	assignValueFunc = reinterpret_cast<void(*)(CatRuntimeContext*, typename TypeTraits<ExpressionT>::functionParameterType)>(functionAddress);
}


template<typename ExpressionT>
inline CatGenericType ExpressionAssignment<ExpressionT>::getExpectedCatType() const
{
	return TypeTraits<ExpressionT>::toGenericType().toWritable();
}
