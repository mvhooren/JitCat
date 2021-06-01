/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ASTHelper.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/Configuration.h"
#include "jitcat/ExpressionErrorManager.h"
#include "ExpressionAssignment.h"

namespace jitcat
{


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
	if (runtimeContext == nullptr)
	{
		runtimeContext = &CatRuntimeContext::getDefaultContext();
	}
	if (Configuration::enableLLVM || JitCat::get()->getHasPrecompiledExpression())
	{
		if (Configuration::enableLLVM || assignValueFunc != &defaultAssignFunction)
		{
			assignValueFunc(runtimeContext, value);
			return !hasError();
		}
	}
	if constexpr (!Configuration::enableLLVM)
	{
		return assignInterpretedValue(runtimeContext, value);
	}
}


template<typename ExpressionT>
inline bool ExpressionAssignment<ExpressionT>::assignInterpretedValue(CatRuntimeContext* runtimeContext, typename TypeTraits<ExpressionT>::functionParameterType value)
{
	if (parseResult.astRootNode != nullptr && parseResult.getNode<AST::CatTypedExpression>()->isAssignable())
	{
		jitcat::AST::CatAssignableExpression* assignable = parseResult.getNode<AST::CatAssignableExpression>();
		std::any target = assignable->executeAssignable(runtimeContext);
		CatGenericType expectedType = getExpectedCatType();
		if constexpr (!std::is_class_v<ExpressionT>  || std::is_enum_v<ExpressionT>)
		{
			std::any anyValue = TypeTraits<ExpressionT>::getCatValue(value);
			jitcat::AST::ASTHelper::doAssignment(target, anyValue, assignable->getAssignableType(), expectedType);
		}
		else
		{
			ExpressionT* targetValue = std::any_cast<ExpressionT*>(target);
			*targetValue = value;
		}
		runtimeContext->clearTemporaries();
		return true;
	}
	return false;
}


template<typename ExpressionT>
inline void ExpressionAssignment<ExpressionT>::compile(CatRuntimeContext* context)
{
	if (context == nullptr)
	{
		context = &CatRuntimeContext::getDefaultContext();
		context->getErrorManager()->clear();
	}
	CatGenericType expectedType = TypeTraits<ExpressionT>::toGenericType();
	if constexpr (std::is_class_v<ExpressionT> || std::is_enum_v<ExpressionT>)
	{
		expectedType = expectedType.toPointer(Reflection::TypeOwnershipSemantics::Value, true, false);
	}
	if (!parse(context, context->getErrorManager(), this, expectedType))
	{
		resetCompiledFunctionToDefault();
	}
}


template<typename ExpressionT>
inline void ExpressionAssignment<ExpressionT>::handleCompiledFunction(uintptr_t functionAddress)
{
	if (functionAddress != 0)
	{
		assignValueFunc = reinterpret_cast<void(*)(CatRuntimeContext*, typename TypeTraits<ExpressionT>::functionParameterType)>(functionAddress);
	}
	else 
	{
		assignValueFunc = &ExpressionAssignment<ExpressionT>::defaultAssignFunction;
	}
}


template<typename ExpressionT>
inline void ExpressionAssignment<ExpressionT>::resetCompiledFunctionToDefault()
{
	assignValueFunc = &defaultAssignFunction;
}


template<typename ExpressionT>
inline CatGenericType ExpressionAssignment<ExpressionT>::getExpectedCatType() const
{
	return TypeTraits<ExpressionT>::toGenericType().toWritable();
}


} //End namespace jitcat
