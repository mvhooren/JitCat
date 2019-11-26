/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#pragma once

#include "jitcat/Expression.h"
#include "jitcat/ExpressionAny.h"
#include "jitcat/ExpressionAssignAny.h"
#include "jitcat/ExpressionAssignment.h"

#include <catch2/catch.hpp>
#include <functional>
#include <iostream>


template<typename T>
inline void checkValueIsEqual(const T& actualValue, const T& expectedValue, bool approximateFloatComparison = true)
{
	if constexpr (std::is_same<T, float>::value)
	{
		if (std::isnan(expectedValue))
		{
			CHECK(std::isnan(actualValue));
		}
		else if (approximateFloatComparison)
		{
			CHECK(actualValue == Approx(expectedValue).epsilon(0.001f));
		}
		else
		{
			CHECK(actualValue == expectedValue);
		}
	}
	else
	{
		CHECK(actualValue == expectedValue);
	}
}


inline bool doCommonChecks(jitcat::ExpressionBase* expression, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, jitcat::CatRuntimeContext& context)
{
	if (!shouldHaveError)
	{
		if (expression->hasError())
		{
			auto errorList = context.getErrorManager()->getErrors();
			REQUIRE(errorList.size() > 0);
			std::cout << errorList[0]->message << "\n";
		}
		REQUIRE_FALSE(expression->hasError());
	}
	else
	{
		REQUIRE(expression->hasError());

		return false;
	}
	if (shouldBeConst)
	{
		CHECK(expression->isConst());
	}
	else
	{
		CHECK_FALSE(expression->isConst());
	}
	if (shouldBeLiteral)
	{
		CHECK(expression->isLiteral());
	}
	else
	{
		CHECK_FALSE(expression->isLiteral());
	}
	return true;
}


template <typename T>
inline void doChecksFn(std::function<bool(const T&)> valueCheck, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, jitcat::Expression<T>& expression, jitcat::CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context))
	{
		CHECK(valueCheck(expression.getValue(&context)));
		CHECK(valueCheck(expression.getInterpretedValue(&context)));
	}
	else if (shouldHaveError)
	{
		if constexpr (!std::is_same<T, void>::value)
		{
			//When te expression has an error, the getValue functions should return the default value
			CHECK(expression.getValue(&context) == T());
			CHECK(expression.getInterpretedValue(&context) == T());
		}
	}
}


template <typename T>
inline void doChecks(const T& expectedValue, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, jitcat::Expression<T>& expression, jitcat::CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context))
	{
		checkValueIsEqual(expression.getValue(&context), expectedValue); 
		checkValueIsEqual(expression.getInterpretedValue(&context), expectedValue); 
	}
	else if (shouldHaveError)
	{
		if constexpr (!std::is_same<T, void>::value)
		{
			//When te expression has an error, the getValue functions should return the default value
			CHECK(expression.getValue(&context) == T());
			CHECK(expression.getInterpretedValue(&context) == T());
		}
	}
}


template <typename T>
inline void doChecks(const T& expectedValue, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, jitcat::ExpressionAny& expression, jitcat::CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context))
	{
		checkValueIsEqual(std::any_cast<T>(expression.getValue(&context)), expectedValue);
		checkValueIsEqual(std::any_cast<T>(expression.getInterpretedValue(&context)), expectedValue);
	}
	else if (shouldHaveError)
	{
		if constexpr (!std::is_same<T, void>::value)
		{
			//When te expression has an error, the getValue functions should return the default value
			CHECK(std::any_cast<T>(expression.getValue(&context)) == T());
			CHECK(std::any_cast<T>(expression.getInterpretedValue(&context)) == T());
		}
	}
}

template <typename T>
inline void checkAssignment(T& assignedValue, const T& expectedValue, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, jitcat::Expression<void>& expression, jitcat::CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context))
	{
		T originalValue = assignedValue;
		expression.getValue(&context);
		checkValueIsEqual(assignedValue, expectedValue, false);
		assignedValue = originalValue;
		expression.getInterpretedValue(&context);
		checkValueIsEqual(assignedValue, expectedValue, false);
		assignedValue = originalValue;
	}
	else if (shouldHaveError)
	{
		if constexpr (!std::is_same<T, void>::value)
		{
			//When te expression has an error, the getValue functions should not crash
			expression.getValue(&context);
			expression.getInterpretedValue(&context);
		}
	}
}


template <typename T>
inline void checkAssignmentCustom(jitcat::Reflection::CustomTypeInstance* instance, const std::string& memberName, const T& expectedValue, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, jitcat::Expression<void>& expression, jitcat::CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context) && instance->getMemberInfo(memberName) != nullptr)
	{
		T originalValue = *instance->getMemberValue<T>(memberName);
		expression.getValue(&context);
		checkValueIsEqual(*instance->getMemberValue<T>(memberName), expectedValue, false);
		instance->setMemberValue(memberName, originalValue);
		expression.getInterpretedValue(&context);
		checkValueIsEqual(*instance->getMemberValue<T>(memberName), expectedValue, false);
		instance->setMemberValue(memberName, originalValue);
	}
	else if (shouldHaveError)
	{
		if constexpr (!std::is_same<T, void>::value)
		{
			//When te expression has an error, the getValue functions should not crash
			expression.getValue(&context);
			expression.getInterpretedValue(&context);
		}
	}
}


template <typename T>
inline void checkAssignExpression(T& assignedValue, const T& newValue, bool shouldHaveError, jitcat::ExpressionAssignment<T>& expression, jitcat::CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, false, false, context))
	{
		T originalValue = assignedValue;
		expression.assignValue(&context, newValue);
		checkValueIsEqual(assignedValue, newValue, false);
		assignedValue = originalValue;
		expression.assignInterpretedValue(&context, newValue);
		checkValueIsEqual(assignedValue, newValue, false);
		assignedValue = originalValue;
	}
	else if (shouldHaveError)
	{
		if constexpr (!std::is_same<T, void>::value)
		{
			//When te expression has an error, the getValue functions should not crash
			expression.assignValue(&context, newValue);
			expression.assignInterpretedValue(&context, newValue);
		}
	}
}


template <typename T>
inline void checkAssignExpressionCustom(jitcat::Reflection::CustomTypeInstance* instance, const std::string& memberName, const T& newValue, bool shouldHaveError, jitcat::ExpressionAssignment<T>& expression, jitcat::CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, false, false, context))
	{
		T originalValue = *instance->getMemberValue<T>(memberName);
		expression.assignValue(&context, newValue);
		checkValueIsEqual(*instance->getMemberValue<T>(memberName), newValue, false);
		instance->setMemberValue(memberName, originalValue);
		expression.assignInterpretedValue(&context, newValue);
		checkValueIsEqual(*instance->getMemberValue<T>(memberName), newValue, false);
		instance->setMemberValue(memberName, originalValue);
	}
	else if (shouldHaveError)
	{
		if constexpr (!std::is_same<T, void>::value)
		{
			//When te expression has an error, the getValue functions should not crash
			expression.assignValue(&context, newValue);
			expression.assignInterpretedValue(&context, newValue);
		}
	}
}


template<typename T>
inline void checkAnyAssignExpression(T& assignedValue, const T& newValue, bool shouldHaveError, jitcat::ExpressionAssignAny& expression, jitcat::CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, false, false, context))
	{
		T originalValue = assignedValue;
		expression.assignValue(&context, newValue, TypeTraits<T>::toGenericType());
		checkValueIsEqual(assignedValue, newValue, false);
		assignedValue = originalValue;
		expression.assignInterpretedValue(&context, newValue, TypeTraits<T>::toGenericType());
		checkValueIsEqual(assignedValue, newValue, false);
		assignedValue = originalValue;

		if constexpr (std::is_pointer<T>::value)
		{
			expression.assignValue(&context, std::any(static_cast<Reflectable*>(newValue)), TypeTraits<T>::toGenericType());
			CHECK(assignedValue == newValue);
			assignedValue = originalValue;

			expression.assignInterpretedValue(&context, std::any(static_cast<Reflectable*>(newValue)), TypeTraits<T>::toGenericType());
			CHECK(assignedValue == newValue);
			assignedValue = originalValue;
		}

	}
	else if (shouldHaveError)
	{
		if constexpr (!std::is_same<T, void>::value)
		{
			//When te expression has an error, the getValue functions should not crash
			expression.assignValue(&context, newValue, TypeTraits<T>::toGenericType());
			expression.assignInterpretedValue(&context, newValue, TypeTraits<T>::toGenericType());
		}
	}
}


template <typename T>
inline void checkAnyAssignExpressionCustom(jitcat::Reflection::CustomTypeInstance* instance, const std::string& memberName, const T& newValue, bool shouldHaveError, jitcat::ExpressionAssignAny& expression, jitcat::CatRuntimeContext & context)
{
	if (doCommonChecks(&expression, shouldHaveError, false, false, context))
	{
		T originalValue = *instance->getMemberValue<T>(memberName);
		expression.assignValue(&context, newValue, TypeTraits<T>::toGenericType());
		checkValueIsEqual(*instance->getMemberValue<T>(memberName), newValue, false);
		instance->setMemberValue(memberName, originalValue);
		expression.assignInterpretedValue(&context, newValue, TypeTraits<T>::toGenericType());
		checkValueIsEqual(*instance->getMemberValue<T>(memberName), newValue, false);
		instance->setMemberValue(memberName, originalValue);
	}
	else if (shouldHaveError)
	{
		if constexpr (!std::is_same<T, void>::value)
		{
			//When te expression has an error, the getValue functions should not crash
			expression.assignValue(&context, newValue, TypeTraits<T>::toGenericType());
			expression.assignInterpretedValue(&context, newValue, TypeTraits<T>::toGenericType());
		}
	}
}