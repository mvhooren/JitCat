/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include <catch2/catch.hpp>
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/CustomTypeInstance.h"
#include "jitcat/CustomTypeMemberInfo.h"
#include "jitcat/Expression.h"
#include "jitcat/ExpressionAny.h"
#include "jitcat/ExpressionAssignment.h"
#include "jitcat/ExpressionAssignAny.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/Tools.h"
#include "TestObjects.h"

#include <functional>
#include <iostream>
#include <cmath>

using namespace jitcat;
using namespace jitcat::LLVM;
using namespace jitcat::Reflection;
using namespace TestObjects;


bool doCommonChecks(ExpressionBase* expression, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, CatRuntimeContext& context)
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
void doChecksFn(std::function<bool(const T&)> valueCheck, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, Expression<T>& expression, CatRuntimeContext& context)
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
void doChecks(const T& expectedValue, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, Expression<T>& expression, CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context))
	{
		if constexpr (std::is_same<T, float>::value)
		{
			CHECK(expression.getValue(&context) == Approx(expectedValue).epsilon(0.001f));
			CHECK(expression.getInterpretedValue(&context) == Approx(expectedValue).epsilon(0.001f));
		}
		else
		{
			CHECK(expression.getValue(&context) == expectedValue);
			CHECK(expression.getInterpretedValue(&context) == expectedValue);
		}
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
void doChecks(const T& expectedValue, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, ExpressionAny& expression, CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context))
	{
		if constexpr (std::is_same<T, float>::value)
		{
			CHECK(std::any_cast<float>(expression.getValue(&context)) == Approx(expectedValue).epsilon(0.001f));
			CHECK(std::any_cast<float>(expression.getInterpretedValue(&context)) == Approx(expectedValue).epsilon(0.001f));
		}
		else
		{
			CHECK(std::any_cast<T>(expression.getValue(&context)) == expectedValue);
			CHECK(std::any_cast<T>(expression.getInterpretedValue(&context)) == expectedValue);
		}
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
void checkAssignment(T& assignedValue, const T& expectedValue, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, Expression<void>& expression, CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context))
	{
		T originalValue = assignedValue;
		expression.getValue(&context);
		CHECK(assignedValue == expectedValue);
		assignedValue = originalValue;
		expression.getInterpretedValue(&context);
		CHECK(assignedValue == expectedValue);
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
void checkAssignmentCustom(CustomTypeInstance* instance, const std::string& memberName, const T& expectedValue, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, Expression<void>& expression, CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context) && instance->getMemberInfo(memberName) != nullptr)
	{
		T originalValue = *instance->getMemberValue<T>(memberName);
		expression.getValue(&context);
		CHECK(*instance->getMemberValue<T>(memberName) == expectedValue);
		instance->setMemberValue(memberName, originalValue);
		expression.getInterpretedValue(&context);
		CHECK(*instance->getMemberValue<T>(memberName) == expectedValue);
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
void checkAssignExpression(T& assignedValue, const T& newValue, bool shouldHaveError, ExpressionAssignment<T>& expression, CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, false, false, context))
	{
		T originalValue = assignedValue;
		expression.assignValue(&context, newValue);
		CHECK(assignedValue == newValue);
		assignedValue = originalValue;
		expression.assignInterpretedValue(&context, newValue);
		CHECK(assignedValue == newValue);
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
void checkAssignExpressionCustom(CustomTypeInstance* instance, const std::string& memberName, const T& newValue, bool shouldHaveError, ExpressionAssignment<T>& expression, CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, false, false, context))
	{
		T originalValue = *instance->getMemberValue<T>(memberName);
		expression.assignValue(&context, newValue);
		CHECK(*instance->getMemberValue<T>(memberName) == newValue);
		instance->setMemberValue(memberName, originalValue);
		expression.assignInterpretedValue(&context, newValue);
		CHECK(*instance->getMemberValue<T>(memberName) == newValue);
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
void checkAnyAssignExpression(T& assignedValue, const T& newValue, bool shouldHaveError, ExpressionAssignAny& expression, CatRuntimeContext& context)
{
	if (doCommonChecks(&expression, shouldHaveError, false, false, context))
	{
		T originalValue = assignedValue;
		expression.assignValue(&context, newValue, TypeTraits<T>::toGenericType());
		CHECK(assignedValue == newValue);
		assignedValue = originalValue;
		expression.assignInterpretedValue(&context, newValue, TypeTraits<T>::toGenericType());
		CHECK(assignedValue == newValue);
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
void checkAnyAssignExpressionCustom(CustomTypeInstance* instance, const std::string& memberName, const T& newValue, bool shouldHaveError, ExpressionAssignAny& expression, CatRuntimeContext & context)
{
	if (doCommonChecks(&expression, shouldHaveError, false, false, context))
	{
		T originalValue = *instance->getMemberValue<T>(memberName);
		expression.assignValue(&context, newValue, TypeTraits<T>::toGenericType());
		CHECK(*instance->getMemberValue<T>(memberName) == newValue);
		instance->setMemberValue(memberName, originalValue);
		expression.assignInterpretedValue(&context, newValue, TypeTraits<T>::toGenericType());
		CHECK(*instance->getMemberValue<T>(memberName) == newValue);
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


TEST_CASE("Regression testing", "[regression]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	reflectedObject.nestedSelfObject->createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("regressionTests", &errorManager);
	context.addScope(&reflectedObject, true);


	SECTION("Crash when using select with object types.")
	{
		Expression<TestObjects::NestedReflectedObject*> testExpression(&context, "select(!nestedSelfObject.getObject2(text, false).no, nestedSelfObject.getObject2(text, false).nestedObject, nestedObjectPointer)");
		doChecks(&reflectedObject.nestedSelfObject->getObject2(reflectedObject.text, false)->nestedObject, false, false, false, testExpression, context);
	}
}


TEST_CASE("Floating Point Tests", "[float][operators]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("floatTests", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Constant")
	{
		Expression<float> testExpression(&context, "42.0f");
		doChecks(42.0f, false, true, true, testExpression, context);
	}
	SECTION("Constant_2")
	{
		Expression<float> testExpression(&context, "42.0");
		doChecks(42.0f, false, true, true, testExpression, context);
	}
	SECTION("Negative Constant")
	{
		Expression<float> testExpression(&context, "-42.0f");
		doChecks(-42.0f, false, true, true, testExpression, context);
	}
	SECTION("Negative Constant_2")
	{
		Expression<float> testExpression(&context, "-42.0");
		doChecks(-42.0f, false, true, true, testExpression, context);
	}
	SECTION("Variable")
	{
		Expression<float> testExpression(&context, "aFloat");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Addition")
	{
		Expression<float> testExpression(&context, "aFloat + 33.3f");
		doChecks(reflectedObject.aFloat + 33.3f, false, false, false, testExpression, context);
	}
	SECTION("Subtraction")
	{
		Expression<float> testExpression(&context, "aFloat - 15.4f");
		doChecks(reflectedObject.aFloat - 15.4f, false, false, false, testExpression, context);
	}
	SECTION("Multiplication")
	{
		Expression<float> testExpression(&context, "aFloat * 22.8f");
		doChecks(reflectedObject.aFloat * 22.8f, false, false, false, testExpression, context);
	}
	SECTION("Division")
	{
		Expression<float> testExpression(&context, "aFloat / 182.0f");
		doChecks(reflectedObject.aFloat / 182.0f, false, false, false, testExpression, context);
	}
	SECTION("Division By Zero")
	{
		Expression<float> testExpression(&context, "aFloat / 0.0f");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Modulo")
	{
		Expression<float> testExpression(&context, "aFloat % 11.5f");
		doChecks(fmodf(reflectedObject.aFloat, 11.5f), false, false, false, testExpression, context);
	}
	SECTION("Modulo_Invalid")
	{
		Expression<float> testExpression(&context, "aFloat % zeroFloat");
		doChecks<float>(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Smaller_1")
	{
		Expression<bool> testExpression(&context, "aFloat < 1000.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Smaller_2")
	{
		Expression<bool> testExpression(&context, "aFloat < 900.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Smaller_3")
	{
		Expression<bool> testExpression(&context, "aFloat < aFloat");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Greater_1")
	{
		Expression<bool> testExpression(&context, "aFloat > 1000.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Greater_2")
	{
		Expression<bool> testExpression(&context, "aFloat > 900.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Greater_3")
	{
		Expression<bool> testExpression(&context, "aFloat > aFloat");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_1")
	{
		Expression<bool> testExpression(&context, "aFloat >= 1000.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_2")
	{
		Expression<bool> testExpression(&context, "aFloat >= 900.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_3")
	{
		Expression<bool> testExpression(&context, "aFloat >= aFloat");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_1")
	{
		Expression<bool> testExpression(&context, "aFloat <= 1000.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_2")
	{
		Expression<bool> testExpression(&context, "aFloat <= 900.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_3")
	{
		Expression<bool> testExpression(&context, "aFloat <= aFloat");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Equals_1")
	{
		Expression<bool> testExpression(&context, "aFloat == 1000.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Equals_2")
	{
		Expression<bool> testExpression(&context, "aFloat == 900.0f");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Equals_3")
	{
		Expression<bool> testExpression(&context, "aFloat == 999.9f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_1")
	{
		Expression<bool> testExpression(&context, "aFloat != 1000.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_2")
	{
		Expression<bool> testExpression(&context, "aFloat != 900.0f");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_3")
	{
		Expression<bool> testExpression(&context, "aFloat != 999.9f");
		doChecks(false, false, false, false, testExpression, context);
	}
}


TEST_CASE("Integer Tests", "[int][operators]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("intTests", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Constant")
	{
		Expression<int> testExpression(&context, "42");
		doChecks(42, false, true, true, testExpression, context);
	}
	SECTION("Negative Constant")
	{
		Expression<int> testExpression(&context, "-42");
		doChecks(-42, false, true, true, testExpression, context);
	}
	SECTION("Variable")
	{
		Expression<int> testExpression(&context, "theInt");
		doChecks(reflectedObject.theInt, false, false, false, testExpression, context);
	}
	SECTION("Addition")
	{
		Expression<int> testExpression(&context, "theInt + 33");
		doChecks(reflectedObject.theInt + 33, false, false, false, testExpression, context);
	}
	SECTION("Subtraction")
	{
		Expression<int> testExpression(&context, "theInt - 15");
		doChecks(reflectedObject.theInt - 15, false, false, false, testExpression, context);
	}
	SECTION("Multiplication")
	{
		Expression<int> testExpression(&context, "theInt * 22");
		doChecks(reflectedObject.theInt * 22, false, false, false, testExpression, context);
	}
	SECTION("Division")
	{
		Expression<int> testExpression(&context, "theInt / 12");
		doChecks(reflectedObject.theInt / 12, false, false, false, testExpression, context);
	}
	SECTION("Division By Zero")
	{
		Expression<int> testExpression(&context, "theInt / 0");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Modulo")
	{
		Expression<int> testExpression(&context, "theInt % 11");
		doChecks(reflectedObject.theInt % 11, false, false, false, testExpression, context);
	}
	SECTION("Modulo_Invalid")
	{
		Expression<int> testExpression(&context, "theInt % 0");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Smaller_1")
	{
		Expression<bool> testExpression(&context, "theInt < 1000");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Smaller_2")
	{
		Expression<bool> testExpression(&context, "theInt < 41");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Smaller_3")
	{
		Expression<bool> testExpression(&context, "theInt < theInt");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Greater_1")
	{
		Expression<bool> testExpression(&context, "theInt > 1000");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Greater_2")
	{
		Expression<bool> testExpression(&context, "theInt > 41");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Greater_3")
	{
		Expression<bool> testExpression(&context, "theInt > theInt");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_1")
	{
		Expression<bool> testExpression(&context, "theInt >= 1000");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_2")
	{
		Expression<bool> testExpression(&context, "theInt >= 40");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_3")
	{
		Expression<bool> testExpression(&context, "theInt >= theInt");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_1")
	{
		Expression<bool> testExpression(&context, "theInt <= 1000");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_2")
	{
		Expression<bool> testExpression(&context, "theInt <= 40");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_3")
	{
		Expression<bool> testExpression(&context, "theInt <= theInt");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Equals_1")
	{
		Expression<bool> testExpression(&context, "theInt == 1000");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Equals_2")
	{
		Expression<bool> testExpression(&context, "theInt == 0");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Equals_3")
	{
		Expression<bool> testExpression(&context, "theInt == 42");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_1")
	{
		Expression<bool> testExpression(&context, "theInt != 1000");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_2")
	{
		Expression<bool> testExpression(&context, "theInt != 40");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NotEquals_3")
	{
		Expression<bool> testExpression(&context, "theInt != 42");
		doChecks(false, false, false, false, testExpression, context);
	}
}


TEST_CASE("Boolean Tests", "[bool][operators]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("intTests", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("True Constant")
	{
		Expression<bool> testExpression(&context, "true");
		doChecks(true, false, true, true, testExpression, context);
	}
	SECTION("False Constant")
	{
		Expression<bool> testExpression(&context, "false");
		doChecks(false, false, true, true, testExpression, context);
	}
	SECTION("Constant Operator And")
	{
		Expression<bool> testExpression(&context, "false && true");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("Constant Operator Or")
	{
		Expression<bool> testExpression(&context, "false || true");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Constant Operator Not")
	{
		Expression<bool> testExpression(&context, "!false");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Constant Operator Equals")
	{
		Expression<bool> testExpression(&context, "false == false");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Constant Operator Not Equals")
	{
		Expression<bool> testExpression(&context, "true != false");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Variable")
	{
		Expression<bool> testExpression(&context, "aBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Not")
	{
		Expression<bool> testExpression(&context, "!aBoolean");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Not")
	{
		Expression<bool> testExpression(&context, "!no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator And")
	{
		Expression<bool> testExpression(&context, "aBoolean && no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator And")
	{
		Expression<bool> testExpression(&context, "aBoolean && !no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator And")
	{
		Expression<bool> testExpression(&context, "!aBoolean && no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Or")
	{
		Expression<bool> testExpression(&context, "aBoolean || no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Or")
	{
		Expression<bool> testExpression(&context, "!aBoolean || !no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Or")
	{
		Expression<bool> testExpression(&context, "!aBoolean || no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Equals")
	{
		Expression<bool> testExpression(&context, "aBoolean == no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Equals")
	{
		Expression<bool> testExpression(&context, "aBoolean == !no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator Equals")
	{
		Expression<bool> testExpression(&context, "!aBoolean == no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator NotEquals")
	{
		Expression<bool> testExpression(&context, "aBoolean != no");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator NotEquals")
	{
		Expression<bool> testExpression(&context, "aBoolean != !no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Variable Operator NotEquals")
	{
		Expression<bool> testExpression(&context, "!aBoolean != no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Addition")
	{
		Expression<bool> testExpression(&context, "aBoolean + 33");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Subtraction")
	{
		Expression<bool> testExpression(&context, "aBoolean - 15");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Multiplication")
	{
		Expression<bool> testExpression(&context, "aBoolean * 22");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Division")
	{
		Expression<bool> testExpression(&context, "aBoolean / 12");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Division By Zero")
	{
		Expression<bool> testExpression(&context, "aBoolean / 0");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Modulo")
	{
		Expression<bool> testExpression(&context, "aBoolean % 11");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Modulo_Invalid")
	{
		Expression<bool> testExpression(&context, "aBoolean % 0");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Smaller_1")
	{
		Expression<bool> testExpression(&context, "aBoolean < 1000");
		doChecks(true, true, false, false, testExpression, context);
	}
	SECTION("Smaller_2")
	{
		Expression<bool> testExpression(&context, "aBoolean < 41");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Smaller_3")
	{
		Expression<bool> testExpression(&context, "aBoolean < theInt");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Greater_1")
	{
		Expression<bool> testExpression(&context, "aBoolean > 1000");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Greater_2")
	{
		Expression<bool> testExpression(&context, "aBoolean > 41");
		doChecks(true, true, false, false, testExpression, context);
	}
	SECTION("Greater_3")
	{
		Expression<bool> testExpression(&context, "aBoolean > theInt");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_1")
	{
		Expression<bool> testExpression(&context, "aBoolean >= 1000");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_2")
	{
		Expression<bool> testExpression(&context, "aBoolean >= 40");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("GreaterOrEqual_3")
	{
		Expression<bool> testExpression(&context, "aBoolean >= theInt");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_1")
	{
		Expression<bool> testExpression(&context, "aBoolean <= 1000");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_2")
	{
		Expression<bool> testExpression(&context, "aBoolean <= 40");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("SmallerOrEqual_3")
	{
		Expression<bool> testExpression(&context, "aBoolean <= theInt");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Equals_1")
	{
		Expression<bool> testExpression(&context, "aBoolean == 1000");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Equals_2")
	{
		Expression<bool> testExpression(&context, "aBoolean == 0");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Equals_3")
	{
		Expression<bool> testExpression(&context, "aBoolean == 42");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("NotEquals_1")
	{
		Expression<bool> testExpression(&context, "aBoolean != 1000");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("NotEquals_2")
	{
		Expression<bool> testExpression(&context, "aBoolean != 40");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("NotEquals_3")
	{
		Expression<bool> testExpression(&context, "aBoolean != 42");
		doChecks(false, true, false, false, testExpression, context);
	}
}


TEST_CASE("Operator precedence", "[operators][precedence]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Select", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Precedence test 1")
	{
		Expression<int> testExpression(&context, "3 + 5 * 11 - 8 / 2");
		doChecks(3 + 5 * 11 - 8 / 2, false, true, false, testExpression, context);
	}
	SECTION("Precedence test 2")
	{
		Expression<bool> testExpression(&context, "false && false || true");
		doChecks(false && false || true, false, true, false, testExpression, context);
	}
	SECTION("Precedence test 3")
	{
		Expression<int> testExpression(&context, "theInt + theInt * largeInt - theInt / 2");
		doChecks(reflectedObject.theInt + reflectedObject.theInt * reflectedObject.largeInt - reflectedObject.theInt / 2, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 4")
	{
		Expression<bool> testExpression(&context, "no && no || aBoolean");
		doChecks(false && false || true, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 5")
	{
		Expression<bool> testExpression(&context, "aBoolean != no || aBoolean");
		doChecks(true != false || true, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 5")
	{
		Expression<bool> testExpression(&context, "aBoolean == no || no");
		doChecks(true == false || false, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 6")
	{
		Expression<bool> testExpression(&context, "theInt < largeInt || no");
		doChecks(42 < 100 || false, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 6")
	{
		Expression<bool> testExpression(&context, "theInt <= largeInt || no");
		doChecks(42 <= 100 || false, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 7")
	{
		Expression<bool> testExpression(&context, "theInt > largeInt || aBoolean");
		doChecks(42 > 100 || true, false, false, false, testExpression, context);
	}
	SECTION("Precedence test 8")
	{
		Expression<bool> testExpression(&context, "theInt >= largeInt || aBoolean");
		doChecks(42 >= 100 || true, false, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToInt", "[builtins][toint]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToInt", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToInt_cc")
	{
		Expression<int> testExpression(&context, "toInt(11.1f)");
		doChecks(11, false, true, false, testExpression, context);
	}
	SECTION("ToInt_float")
	{
		Expression<int> testExpression(&context, "toInt(aFloat)");
		doChecks((int)reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("ToInt_int")
	{
		Expression<int> testExpression(&context, "toInt(theInt)");
		doChecks(reflectedObject.theInt, false, false, false, testExpression, context);
	}
	SECTION("ToInt_bool")
	{
		Expression<int> testExpression(&context, "toInt(aBoolean)");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("ToInt_stringConst")
	{
		Expression<int> testExpression(&context, "toInt(\"10\")");
		doChecks(10, false, true, false, testExpression, context);
	}
	SECTION("ToInt_string")
	{
		Expression<int> testExpression(&context, "toInt(numberString)");
		doChecks(123, false, false, false, testExpression, context);
	}
	SECTION("ToInt_string2")
	{
		Expression<int> testExpression(&context, "toInt(text)");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("ToInt_noarg")
	{
		Expression<int> testExpression(&context, "toInt()");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("ToInt_morearg")
	{
		Expression<int> testExpression(&context, "toInt(theInt, aFloat)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("ToInt_obj")
	{
		Expression<int> testExpression(&context, "toInt(nestedObject)");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToFloat", "[builtins][tofloat]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToFloat", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToFloat_cc")
	{
		Expression<float> testExpression(&context, "toFloat(11.1f)");
		doChecks(11.1f, false, true, false, testExpression, context);
	}
	SECTION("ToFloat_float")
	{
		Expression<float> testExpression(&context, "toFloat(aFloat)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("ToFloat_int")
	{
		Expression<float> testExpression(&context, "toFloat(theInt)");
		doChecks((float)reflectedObject.theInt, false, false, false, testExpression, context);
	}
	SECTION("ToFloat_bool")
	{
		Expression<float> testExpression(&context, "toFloat(aBoolean)");
		doChecks(1.0f, false, false, false, testExpression, context);
	}
	SECTION("ToFloat_stringConst")
	{
		Expression<float> testExpression(&context, "toFloat(\"10\")");
		doChecks(10.0f, false, true, false, testExpression, context);
	}
	SECTION("ToFloat_string")
	{
		Expression<float> testExpression(&context, "toFloat(numberString)");
		doChecks(123.4f, false, false, false, testExpression, context);
	}
	SECTION("ToFloat_string2")
	{
		Expression<float> testExpression(&context, "toFloat(text)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("ToFloat_noarg")
	{
		Expression<float> testExpression(&context, "toFloat()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("ToFloat_morearg")
	{
		Expression<float> testExpression(&context, "toFloat(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("ToFloat_obj")
	{
		Expression<float> testExpression(&context, "toFloat(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToString", "[builtins][tostring]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToString_cc")
	{
		Expression<std::string> testExpression(&context, "toString(11.1f)");
		doChecks(Tools::makeString(11.1f), false, true, false, testExpression, context);
	}
	SECTION("ToString_float")
	{
		Expression<std::string> testExpression(&context, "toString(aFloat)");
		doChecks(Tools::makeString(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("ToString_int")
	{
		Expression<std::string> testExpression(&context, "toString(theInt)");
		doChecks(Tools::makeString(reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("ToString_bool")
	{
		Expression<std::string> testExpression(&context, "toString(aBoolean)");
		doChecks(std::string("1"), false, false, false, testExpression, context);
	}
	SECTION("ToString_stringConst")
	{
		Expression<std::string> testExpression(&context, "toString(\"10\")");
		doChecks(std::string("10"), false, true, false, testExpression, context);
	}
	SECTION("ToString_string")
	{
		Expression<std::string> testExpression(&context, "toString(numberString)");
		doChecks(reflectedObject.numberString, false, false, false, testExpression, context);
	}
	SECTION("ToString_string2")
	{
		Expression<std::string> testExpression(&context, "toString(text)");
		doChecks(reflectedObject.text, false, false, false, testExpression, context);
	}
	SECTION("ToString_noarg")
	{
		Expression<std::string> testExpression(&context, "toString()");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToString_morearg")
	{
		Expression<std::string> testExpression(&context, "toString(theInt, aFloat)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToString_obj")
	{
		Expression<std::string> testExpression(&context, "toString(nestedObject)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToPrettyString", "[builtins][toprettystring]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToPrettyString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToPrettyString_cc")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(11.1f)");
		doChecks(Tools::makeString(11.1f), false, true, false, testExpression, context);
	}
	SECTION("ToPrettyString_float")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(aFloat)");
		doChecks(Tools::makeString(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_int")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(largeInt)");
		doChecks(LLVMCatIntrinsics::intToPrettyString(reflectedObject.largeInt), false, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_bool")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(aBoolean)");
		doChecks(std::string("1"), false, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_stringConst")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(\"10\")");
		doChecks(std::string("10"), false, true, false, testExpression, context);
	}
	SECTION("ToPrettyString_string")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(numberString)");
		doChecks(reflectedObject.numberString, false, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_string2")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(text)");
		doChecks(reflectedObject.text, false, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_noarg")
	{
		Expression<std::string> testExpression(&context, "toPrettyString()");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_morearg")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(theInt, aFloat)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToPrettyString_obj")
	{
		Expression<std::string> testExpression(&context, "toPrettyString(nestedObject)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToFixedLengthString", "[builtins][tofixedlengthstring]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToFixedLengthString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToFixedLengthString_cc")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(11.1f, 10)");
		doChecks(Tools::makeString(11.1f), true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_float")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(aFloat, 10)");
		doChecks(Tools::makeString(reflectedObject.aFloat), true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_int")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(largeInt, 10)");
		doChecks(LLVMCatIntrinsics::intToFixedLengthString(reflectedObject.largeInt, 10), false, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_bool")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(aBoolean, 10)");
		doChecks(std::string("1"), true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_stringConst")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(\"10\", 10)");
		doChecks(std::string("10"), true, true, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_string")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(numberString, 10)");
		doChecks(reflectedObject.numberString, true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_string2")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(text, 10)");
		doChecks(reflectedObject.text, true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_noarg")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString()");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_morearg")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(theInt, aFloat, 10)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
	SECTION("ToFixedLengthString_obj")
	{
		Expression<std::string> testExpression(&context, "toFixedLengthString(nestedObject, 10)");
		doChecks(std::string(), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ToBool", "[builtins][tobool]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ToBool", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ToBool_cc")
	{
		Expression<bool> testExpression(&context, "toBool(11.1f)");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("ToBool_cc")
	{
		Expression<bool> testExpression(&context, "toBool(-11.1f)");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("ToBool_cc")
	{
		Expression<bool> testExpression(&context, "toBool(0.0f)");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("ToBool_float")
	{
		Expression<bool> testExpression(&context, "toBool(aFloat)");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("ToBool_int")
	{
		Expression<bool> testExpression(&context, "toBool(theInt)");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("ToBool_int")
	{
		Expression<bool> testExpression(&context, "toBool(-theInt)");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("ToBool_int")
	{
		Expression<bool> testExpression(&context, "toBool(0)");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("ToBool_bool")
	{
		Expression<bool> testExpression(&context, "toBool(aBoolean)");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("ToBool_bool")
	{
		Expression<bool> testExpression(&context, "toBool(false)");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("ToBool_stringConst")
	{
		Expression<bool> testExpression(&context, "toBool(\"10\")");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("ToBool_stringConst")
	{
		Expression<bool> testExpression(&context, "toBool(\"\")");
		doChecks(false, false, true, false, testExpression, context);
	}
	SECTION("ToBool_string")
	{
		Expression<bool> testExpression(&context, "toBool(numberString)");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("ToBool_string2")
	{
		Expression<bool> testExpression(&context, "toBool(text)");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("ToBool_noarg")
	{
		Expression<bool> testExpression(&context, "toBool()");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("ToBool_morearg")
	{
		Expression<bool> testExpression(&context, "toBool(theInt, aFloat)");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("ToBool_obj")
	{
		Expression<bool> testExpression(&context, "toBool(nestedObject)");
		doChecks(false, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Sin", "[builtins][sin]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Sin", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Sin_Constant")
	{
		Expression<float> testExpression(&context, "sin(42.0f)");
		doChecks<float>(sin(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sin_Negative Constant")
	{
		Expression<float> testExpression(&context, "sin(-42.0f)");
		doChecks<float>(sin(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sin_IntConstant")
	{
		Expression<float> testExpression(&context, "sin(3)");
		doChecks<float>(sin((float)3), false, true, false, testExpression, context);
	}
	SECTION("Sin_Variable")
	{
		Expression<float> testExpression(&context, "sin(aFloat)");
		doChecks<float>(sin(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sin_Negative Variable")
	{
		Expression<float> testExpression(&context, "sin(-aFloat)");
		doChecks<float>(sin(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sin_IntVariable")
	{
		Expression<float> testExpression(&context, "sin(theInt)");
		doChecks<float>(sin((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Sin_string")
	{
		Expression<float> testExpression(&context, "sin(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sin_noarg")
	{
		Expression<float> testExpression(&context, "sin()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sin_morearg")
	{
		Expression<float> testExpression(&context, "sin(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sin_obj")
	{
		Expression<float> testExpression(&context, "sin(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Cos", "[builtins][cos]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Cos", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Cos_Constant")
	{
		Expression<float> testExpression(&context, "cos(42.0f)");
		doChecks<float>(cos(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Cos_Negative Constant")
	{
		Expression<float> testExpression(&context, "cos(-42.0f)");
		doChecks<float>(cos(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Cos_IntConstant")
	{
		Expression<float> testExpression(&context, "cos(3)");
		doChecks<float>(cos((float)3), false, true, false, testExpression, context);
	}
	SECTION("Cos_Variable")
	{
		Expression<float> testExpression(&context, "cos(aFloat)");
		doChecks<float>(cos(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Cos_Negative Variable")
	{
		Expression<float> testExpression(&context, "cos(-aFloat)");
		doChecks<float>(cos(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Cos_IntVariable")
	{
		Expression<float> testExpression(&context, "cos(theInt)");
		doChecks<float>(cos((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Cos_string")
	{
		Expression<float> testExpression(&context, "cos(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cos_noarg")
	{
		Expression<float> testExpression(&context, "cos()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cos_morearg")
	{
		Expression<float> testExpression(&context, "cos(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cos_obj")
	{
		Expression<float> testExpression(&context, "cos(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Tan", "[builtins][tan]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Tan", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Tan_Constant")
	{
		Expression<float> testExpression(&context, "tan(42.0f)");
		doChecks<float>(tan(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Tan_Negative Constant")
	{
		Expression<float> testExpression(&context, "tan(-42.0f)");
		doChecks<float>(tan(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Tan_IntConstant")
	{
		Expression<float> testExpression(&context, "tan(3)");
		doChecks<float>(tan((float)3), false, true, false, testExpression, context);
	}
	SECTION("Tan_Variable")
	{
		Expression<float> testExpression(&context, "tan(aFloat)");
		doChecks<float>(tan(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Tan_Negative Variable")
	{
		Expression<float> testExpression(&context, "tan(-aFloat)");
		doChecks<float>(tan(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Tan_IntVariable")
	{
		Expression<float> testExpression(&context, "tan(theInt)");
		doChecks<float>(tan((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Tan_string")
	{
		Expression<float> testExpression(&context, "tan(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Tan_noarg")
	{
		Expression<float> testExpression(&context, "tan()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Tan_morearg")
	{
		Expression<float> testExpression(&context, "tan(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Tan_obj")
	{
		Expression<float> testExpression(&context, "abs(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Abs", "[builtins][abs]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Abs", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Abs_Constant")
	{
		Expression<float> testExpression(&context, "abs(42.0f)");
		doChecks<float>(abs(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Abs_Negative Constant")
	{
		Expression<float> testExpression(&context, "abs(-42.0f)");
		doChecks<float>(abs(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Abs_IntConstant")
	{
		Expression<int> testExpression(&context, "abs(3)");
		doChecks(abs(3), false, true, false, testExpression, context);
	}
	SECTION("Abs_Negative IntConstant")
	{
		Expression<int> testExpression(&context, "abs(-3)");
		doChecks(abs(-3), false, true, false, testExpression, context);
	}
	SECTION("Abs_Variable")
	{
		Expression<int> testExpression(&context, "abs(-33)");
		doChecks(abs(-33), false, true, false, testExpression, context);
	}
	SECTION("Abs_Negative Variable")
	{
		Expression<float> testExpression(&context, "abs(-aFloat)");
		doChecks<float>(abs(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Abs_Zero Variable")
	{
		Expression<float> testExpression(&context, "abs(zeroFloat)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Abs_IntVariable")
	{
		Expression<int> testExpression(&context, "abs(theInt)");
		doChecks(abs(reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Abs_Negative IntVariable")
	{
		Expression<int> testExpression(&context, "abs(-theInt)");
		doChecks(abs(-reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Abs_string")
	{
		Expression<int> testExpression(&context, "abs(numberString)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Abs_noarg")
	{
		Expression<int> testExpression(&context, "abs()");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Abs_morearg")
	{
		Expression<int> testExpression(&context, "abs(theInt, aFloat)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Abs_obj")
	{
		Expression<int> testExpression(&context, "abs(nestedObject)");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Sqrt", "[builtins][sqrt]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Sqrt", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Sqrt_Constant")
	{
		Expression<float> testExpression(&context, "sqrt(42.0f)");
		doChecks<float>(sqrt(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sqrt_Negative Constant")
	{
		Expression<float> testExpression(&context, "sqrt(-42.0f)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, true, false, testExpression, context);
	}
	SECTION("Sqrt_IntConstant")
	{
		Expression<float> testExpression(&context, "sqrt(3)");
		doChecks<float>(sqrt(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Sqrt_Negative IntConstant")
	{
		Expression<float> testExpression(&context, "sqrt(-3)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, true, false, testExpression, context);
	}
	SECTION("Sqrt_Zero Variable")
	{
		Expression<float> testExpression(&context, "sqrt(zeroFloat)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Sqrt_Variable")
	{
		Expression<float> testExpression(&context, "sqrt(aFloat)");
		doChecks<float>(sqrt(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sqrt_Negative Variable")
	{
		Expression<float> testExpression(&context, "sqrt(-aFloat)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, false, false, testExpression, context);
	}
	SECTION("Sqrt_IntVariable")
	{
		Expression<float> testExpression(&context, "sqrt(theInt)");
		doChecks<float>(sqrt((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Sqrt_Negative IntVariable")
	{
		Expression<float> testExpression(&context, "sqrt(-theInt)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, false, false, testExpression, context);
	}
	SECTION("Sqrt_string")
	{
		Expression<float> testExpression(&context, "sqrt(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sqrt_noarg")
	{
		Expression<float> testExpression(&context, "sqrt()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sqrt_morearg")
	{
		Expression<float> testExpression(&context, "sqrt(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Sqrt_obj")
	{
		Expression<float> testExpression(&context, "sqrt(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Random", "[builtins][rand]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Random", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Random")
	{
		Expression<float> testExpression(&context, "rand()");
		doChecksFn<float>([](float value){return value >= 0.0f && value <= 1.0f;}, false, false, false, testExpression, context);
	}
	SECTION("Random_morearg")
	{
		Expression<float> testExpression(&context, "rand(theInt)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Random_obj")
	{
		Expression<float> testExpression(&context, "rand(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: RandomRange", "[builtins][rand]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_RandomRange", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("RandomRange_float_constant")
	{
		Expression<float> testExpression(&context, "rand(101.0f, 102.0f)");
		doChecksFn<float>([&](float value){return value >= 101.0f && value <= 102.0f;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_float_negative_constant")
	{
		Expression<float> testExpression(&context, "rand(-101.0f, -102.0f)");
		doChecksFn<float>([&](float value){return value >= -102.0f && value <= -101.0f;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_float")
	{
		Expression<float> testExpression(&context, "rand(-aFloat, aFloat)");
		doChecksFn<float>([&](float value){return value >= -reflectedObject.aFloat && value <= reflectedObject.aFloat;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_float_same")
	{
		Expression<float> testExpression(&context, "rand(aFloat, aFloat)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_float_same_negative")
	{
		Expression<float> testExpression(&context, "rand(-aFloat, -aFloat)");
		doChecks(-reflectedObject.aFloat, false, false, false, testExpression, context);
	}

	SECTION("RandomRange_int_constant")
	{
		Expression<int> testExpression(&context, "rand(99, 100)");
		doChecksFn<int>([&](int value){return value >= 99 && value <= 100;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_int_negative_constant")
	{
		Expression<int> testExpression(&context, "rand(-99, -100)");
		doChecksFn<int>([&](int value){return value >= -100 && value <= -99;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_int")
	{
		Expression<int> testExpression(&context, "rand(-theInt, theInt)");
		doChecksFn<int>([&](int value){return value >= -reflectedObject.theInt && value <= reflectedObject.theInt;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_int_same")
	{
		Expression<int> testExpression(&context, "rand(theInt, theInt)");
		doChecks(reflectedObject.theInt, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_int_negative_same")
	{
		Expression<int> testExpression(&context, "rand(-theInt, -theInt)");
		doChecks(-reflectedObject.theInt, false, false, false, testExpression, context);
	}


	SECTION("RandomRange_bool")
	{
		//This test probably always succeeds
		Expression<bool> testExpression(&context, "rand(aBoolean, !aBoolean)");
		doChecksFn<bool>([&](bool value){return value == true || value == false;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_bool")
	{
		//This test probably always succeeds
		Expression<bool> testExpression(&context, "rand(!aBoolean, aBoolean)");
		doChecksFn<bool>([&](bool value){return value == true || value == false;}, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_bool_same")
	{
		Expression<bool> testExpression(&context, "rand(aBoolean, aBoolean)");
		doChecks(reflectedObject.aBoolean, false, false, false, testExpression, context);
	}
	SECTION("RandomRange_int_negative_same")
	{
		Expression<bool> testExpression(&context, "rand(!aBoolean, !aBoolean)");
		doChecks(!reflectedObject.aBoolean, false, false, false, testExpression, context);
	}

	SECTION("Random_strings")
	{
		Expression<bool> testExpression(&context, "rand(text, numberString)");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Random_morearg")
	{
		Expression<bool> testExpression(&context, "rand(theInt, aFloat, aFloat)");
		doChecks(false, true, false, false, testExpression, context);
	}
	SECTION("Random_obj")
	{
		Expression<bool> testExpression(&context, "rand(nestedObject, nestedObject)");
		doChecks(false, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Round", "[builtins][round]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Round", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Round_cc1")
	{
		Expression<float> testExpression(&context, "round(11.1f, 1)");
		doChecks(11.1f, false, true, false, testExpression, context);
	}
	SECTION("Round_cc2")
	{
		Expression<float> testExpression(&context, "round(11.1f, 0)");
		doChecks(11.0f, false, true, false, testExpression, context);
	}
	SECTION("Round_cc3")
	{
		Expression<float> testExpression(&context, "round(11.1f, 2)");
		doChecks(11.1f, false, true, false, testExpression, context);
	}
	SECTION("Round_cc4")
	{
		Expression<float> testExpression(&context, "round(-11.1f, 1)");
		doChecks(-11.1f, false, true, false, testExpression, context);
	}
	SECTION("Round_cc5")
	{
		Expression<float> testExpression(&context, "round(-11.1f, 0)");
		doChecks(-11.0f, false, true, false, testExpression, context);
	}
	SECTION("Round_float1")
	{
		Expression<float> testExpression(&context, "round(aFloat, 0)");
		doChecks(LLVMCatIntrinsics::roundFloat(reflectedObject.aFloat, 0), false, false, false, testExpression, context);
	}
	SECTION("Round_float2")
	{
		Expression<float> testExpression(&context, "round(aFloat, 1.0f)");
		doChecks(LLVMCatIntrinsics::roundFloat(reflectedObject.aFloat, 1), false, false, false, testExpression, context);
	}
	SECTION("Round_float3")
	{
		Expression<float> testExpression(&context, "round(aFloat, 2)");
		doChecks(LLVMCatIntrinsics::roundFloat(reflectedObject.aFloat, 1), false, false, false, testExpression, context);
	}
	SECTION("Round_int")
	{
		Expression<float> testExpression(&context, "round(largeInt, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_bool")
	{
		Expression<float> testExpression(&context, "round(aBoolean, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_stringConst")
	{
		Expression<float> testExpression(&context, "round(\"10\", 10)");
		doChecks(0.0f, true, true, false, testExpression, context);
	}
	SECTION("Round_string")
	{
		Expression<float> testExpression(&context, "round(numberString, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_string2")
	{
		Expression<float> testExpression(&context, "round(text, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_noarg")
	{
		Expression<float> testExpression(&context, "round()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_morearg")
	{
		Expression<float> testExpression(&context, "round(theInt, aFloat, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Round_obj")
	{
		Expression<float> testExpression(&context, "round(nestedObject, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: StringRound", "[builtins][stringround]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_StringRound", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("StringRound_cc1")
	{
		Expression<std::string> testExpression(&context, "stringRound(11.1f, 1)");
		doChecks(std::string("11.1"), false, true, false, testExpression, context);
	}
	SECTION("StringRound_cc2")
	{
		Expression<std::string> testExpression(&context, "stringRound(11.1f, 0)");
		doChecks(std::string("11"), false, true, false, testExpression, context);
	}
	SECTION("StringRound_cc3")
	{
		Expression<std::string> testExpression(&context, "stringRound(11.1f, 2)");
		doChecks(std::string("11.1"), false, true, false, testExpression, context);
	}
	SECTION("Round_cc4")
	{
		Expression<std::string> testExpression(&context, "stringRound(-11.1f, 1)");
		doChecks(std::string("-11.1"), false, true, false, testExpression, context);
	}
	SECTION("Round_cc5")
	{
		Expression<std::string> testExpression(&context, "stringRound(-11.1f, 0)");
		doChecks(std::string("-11"), false, true, false, testExpression, context);
	}
	SECTION("StringRound_float1")
	{
		Expression<std::string> testExpression(&context, "stringRound(aFloat, 0)");
		doChecks(LLVMCatIntrinsics::roundFloatToString(reflectedObject.aFloat, 0), false, false, false, testExpression, context);
	}
	SECTION("StringRound_float2")
	{
		Expression<std::string> testExpression(&context, "stringRound(aFloat, 1.0f)");
		doChecks(LLVMCatIntrinsics::roundFloatToString(reflectedObject.aFloat, 1), false, false, false, testExpression, context);
	}
	SECTION("StringRound_float3")
	{
		Expression<std::string> testExpression(&context, "stringRound(aFloat, 2)");
		doChecks(LLVMCatIntrinsics::roundFloatToString(reflectedObject.aFloat, 1), false, false, false, testExpression, context);
	}
	SECTION("StringRound_int")
	{
		Expression<std::string> testExpression(&context, "stringRound(largeInt, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_bool")
	{
		Expression<std::string> testExpression(&context, "stringRound(aBoolean, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_stringConst")
	{
		Expression<std::string> testExpression(&context, "stringRound(\"10\", 10)");
		doChecks(std::string(""), true, true, false, testExpression, context);
	}
	SECTION("StringRound_string")
	{
		Expression<std::string> testExpression(&context, "stringRound(numberString, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_string2")
	{
		Expression<std::string> testExpression(&context, "stringRound(text, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_noarg")
	{
		Expression<std::string> testExpression(&context, "stringRound()");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_morearg")
	{
		Expression<std::string> testExpression(&context, "stringRound(theInt, aFloat, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("StringRound_obj")
	{
		Expression<std::string> testExpression(&context, "stringRound(nestedObject, 10)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Cap", "[builtins][cap]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Cap", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Cap_cc1")
	{
		Expression<float> testExpression(&context, "cap(11.1f, 1, 11)");
		doChecks(11.0f, false, true, false, testExpression, context);
	}
	SECTION("Cap_cc2")
	{
		Expression<int> testExpression(&context, "cap(11, 20, 12)");
		doChecks(12, false, true, false, testExpression, context);
	}
	SECTION("Cap_cc3")
	{
		Expression<float> testExpression(&context, "cap(-11.1f, -11.2, -11)");
		doChecks(-11.1f, false, true, false, testExpression, context);
	}
	SECTION("Cap_cc4")
	{
		Expression<int> testExpression(&context, "cap(-100, -98, -99)");
		doChecks(-99, false, true, false, testExpression, context);
	}
	SECTION("Cap_cc5")
	{
		Expression<int> testExpression(&context, "cap(5, -4.0f, 6.0f)");
		doChecks(5, false, true, false, testExpression, context);
	}
	SECTION("Cap_float1")
	{
		Expression<float> testExpression(&context, "cap(aFloat, 0, 1000)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Cap_float2")
	{
		Expression<float> testExpression(&context, "cap(aFloat, 1.0f, 10)");
		doChecks(10.0f, false, false, false, testExpression, context);
	}
	SECTION("Cap_float3")
	{
		Expression<float> testExpression(&context, "cap(-aFloat, 0.0f, -999.0f)");
		doChecks(-999.0f, false, false, false, testExpression, context);
	}
	SECTION("Cap_int1")
	{
		Expression<int> testExpression(&context, "cap(largeInt, 0, 1)");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("Cap_int2")
	{
		Expression<int> testExpression(&context, "cap(-largeInt, 0, 1)");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Cap_int3")
	{
		Expression<int> testExpression(&context, "cap(largeInt, largeInt, largeInt)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Cap_int4")
	{
		Expression<int> testExpression(&context, "cap(-largeInt, -largeInt, -largeInt)");
		doChecks(-reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Cap_int4")
	{
		Expression<int> testExpression(&context, "cap(largeInt, largeInt + 1, largeInt - 1)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Cap_bool")
	{
		Expression<int> testExpression(&context, "cap(aBoolean, 0, 1)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Cap_stringConst")
	{
		Expression<int> testExpression(&context, "cap(\"10\", 10, 11)");
		doChecks(0, true, true, false, testExpression, context);
	}
	SECTION("Cap_string")
	{
		Expression<float> testExpression(&context, "cap(numberString, 10.0f, 11.0f)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cap_string2")
	{
		Expression<int> testExpression(&context, "cap(text, 10, 12)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Cap_noarg")
	{
		Expression<float> testExpression(&context, "cap()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cap_fewerarg")
	{
		Expression<float> testExpression(&context, "cap(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cap_morearg")
	{
		Expression<float> testExpression(&context, "cap(theInt, aFloat, 10, 11)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Cap_obj")
	{
		Expression<float> testExpression(&context, "cap(nestedObject, 10, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Min", "[builtins][min]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Min", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Min_cc1")
	{
		Expression<float> testExpression(&context, "min(11.1f, 1)");
		doChecks(1.0f, false, true, false, testExpression, context);
	}
	SECTION("Min_cc2")
	{
		Expression<int> testExpression(&context, "min(0, 11.0f)");
		doChecks(0, false, true, false, testExpression, context);
	}
	SECTION("Min_cc3")
	{
		Expression<int> testExpression(&context, "min(9, -9)");
		doChecks(-9, false, true, false, testExpression, context);
	}
	SECTION("Min_cc4")
	{
		Expression<float> testExpression(&context, "min(-11.1f, 999.0f)");
		doChecks(-11.1f, false, true, false, testExpression, context);
	}
	SECTION("Min_float1")
	{
		Expression<float> testExpression(&context, "min(aFloat, 0)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Min_float2")
	{
		Expression<float> testExpression(&context, "min(aFloat, 1000.0f)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Min_float3")
	{
		Expression<float> testExpression(&context, "min(-aFloat, aFloat)");
		doChecks(-reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Min_int1")
	{
		Expression<int> testExpression(&context, "min(largeInt, 10)");
		doChecks(10, false, false, false, testExpression, context);
	}
	SECTION("Min_int2")
	{
		Expression<int> testExpression(&context, "min(largeInt, -largeInt)");
		doChecks(-reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Min_int3")
	{
		Expression<int> testExpression(&context, "min(largeInt, largeInt)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Min_int4")
	{
		Expression<int> testExpression(&context, "min(largeInt, largeInt + 1)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Min_bool")
	{
		Expression<float> testExpression(&context, "min(aBoolean, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Min_stringConst")
	{
		Expression<float> testExpression(&context, "min(\"10\", 10)");
		doChecks(0.0f, true, true, false, testExpression, context);
	}
	SECTION("Min_string")
	{
		Expression<float> testExpression(&context, "min(numberString, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Min_string2")
	{
		Expression<float> testExpression(&context, "min(text, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Min_noarg")
	{
		Expression<float> testExpression(&context, "min()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Min_morearg")
	{
		Expression<float> testExpression(&context, "min(theInt, aFloat, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Min_obj")
	{
		Expression<float> testExpression(&context, "min(nestedObject, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Max", "[builtins][max]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Max", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Max_cc1")
	{
		Expression<float> testExpression(&context, "max(11.1f, 1)");
		doChecks(11.1f, false, true, false, testExpression, context);
	}
	SECTION("Max_cc2")
	{
		Expression<int> testExpression(&context, "max(0, 11.0f)");
		doChecks(11, false, true, false, testExpression, context);
	}
	SECTION("Max_cc3")
	{
		Expression<int> testExpression(&context, "max(9, -9)");
		doChecks(9, false, true, false, testExpression, context);
	}
	SECTION("Max_cc4")
	{
		Expression<float> testExpression(&context, "max(-11.1f, 999.0f)");
		doChecks(999.0f, false, true, false, testExpression, context);
	}
	SECTION("Max_float1")
	{
		Expression<float> testExpression(&context, "max(aFloat, 0)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Max_float2")
	{
		Expression<float> testExpression(&context, "max(aFloat, 1000.0f)");
		doChecks(1000.0f, false, false, false, testExpression, context);
	}
	SECTION("Max_float3")
	{
		Expression<float> testExpression(&context, "max(-aFloat, aFloat)");
		doChecks(reflectedObject.aFloat, false, false, false, testExpression, context);
	}
	SECTION("Max_int1")
	{
		Expression<int> testExpression(&context, "max(largeInt, 10)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Max_int2")
	{
		Expression<int> testExpression(&context, "max(largeInt, -largeInt)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Max_int3")
	{
		Expression<int> testExpression(&context, "max(largeInt, largeInt)");
		doChecks(reflectedObject.largeInt, false, false, false, testExpression, context);
	}
	SECTION("Max_int4")
	{
		Expression<int> testExpression(&context, "max(largeInt, largeInt + 1)");
		doChecks(reflectedObject.largeInt + 1, false, false, false, testExpression, context);
	}
	SECTION("Max_bool")
	{
		Expression<float> testExpression(&context, "max(aBoolean, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Max_stringConst")
	{
		Expression<float> testExpression(&context, "max(\"10\", 10)");
		doChecks(0.0f, true, true, false, testExpression, context);
	}
	SECTION("Max_string")
	{
		Expression<float> testExpression(&context, "max(numberString, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Max_string2")
	{
		Expression<float> testExpression(&context, "max(text, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Max_noarg")
	{
		Expression<float> testExpression(&context, "max()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Max_morearg")
	{
		Expression<float> testExpression(&context, "max(theInt, aFloat, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Max_obj")
	{
		Expression<float> testExpression(&context, "max(nestedObject, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Log", "[builtins][log]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Log", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Log_Constant")
	{
		Expression<float> testExpression(&context, "log(42.0f)");
		doChecks(log10f(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Log_Negative Constant")
	{
		Expression<float> testExpression(&context, "log(-42.0f)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, true, false, testExpression, context);
	}
	SECTION("Log_IntConstant")
	{
		Expression<float> testExpression(&context, "log(3)");
		doChecks(log10f(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Log_Negative IntConstant")
	{
		Expression<float> testExpression(&context, "log(-3)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, true, false, testExpression, context);
	}
	SECTION("Log_Zero Variable")
	{
		Expression<float> testExpression(&context, "log(zeroFloat)");
		doChecks(-std::numeric_limits<float>::infinity(), false, false, false, testExpression, context);
	}
	SECTION("Log_Variable")
	{
		Expression<float> testExpression(&context, "log(aFloat)");
		doChecks(log10f(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Log_Negative Variable")
	{
		Expression<float> testExpression(&context, "log(-aFloat)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, false, false, testExpression, context);
	}
	SECTION("Log_IntVariable")
	{
		Expression<float> testExpression(&context, "log(theInt)");
		doChecks(log10f((float)reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Log_Negative IntVariable")
	{
		Expression<float> testExpression(&context, "log(-theInt)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, false, false, testExpression, context);
	}
	SECTION("Log_string")
	{
		Expression<float> testExpression(&context, "log(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Log_noarg")
	{
		Expression<float> testExpression(&context, "log()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Log_morearg")
	{
		Expression<float> testExpression(&context, "log(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Log_obj")
	{
		Expression<float> testExpression(&context, "log(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Pow", "[builtins][pow]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Pow", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Pow_cc1")
	{
		Expression<float> testExpression(&context, "pow(11.1f, 1)");
		doChecks<float>(pow(11.1f, 1), false, true, false, testExpression, context);
	}
	SECTION("Pow_cc2")
	{
		Expression<float> testExpression(&context, "pow(0, 11.0f)");
		doChecks(0.0f, false, true, false, testExpression, context);
	}
	SECTION("Pow_cc3")
	{
		Expression<float> testExpression(&context, "pow(9, -9)");
		doChecks((float)pow(9, -9), false, true, false, testExpression, context);
	}
	SECTION("Pow_cc4")
	{
		Expression<float> testExpression(&context, "pow(-11.1f, 999.0f)");
		doChecks<float>(pow(-11.1f, 999.0f), false, true, false, testExpression, context);
	}
	SECTION("Pow_float1")
	{
		Expression<float> testExpression(&context, "pow(aFloat, 0)");
		doChecks(1.0f, false, false, false, testExpression, context);
	}
	SECTION("Pow_float2")
	{
		Expression<float> testExpression(&context, "pow(aFloat, 1000.0f)");
		doChecks<float>(pow(reflectedObject.aFloat, 1000.0f), false, false, false, testExpression, context);
	}
	SECTION("Pow_float3")
	{
		Expression<float> testExpression(&context, "pow(-aFloat, aFloat)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, false, false, testExpression, context);
	}
	SECTION("Pow_int1")
	{
		Expression<float> testExpression(&context, "pow(largeInt, 10)");
		doChecks((float)pow(reflectedObject.largeInt, 10), false, false, false, testExpression, context);
	}
	SECTION("Pow_int2")
	{
		Expression<float> testExpression(&context, "pow(largeInt, -largeInt)");
		doChecks((float)pow(reflectedObject.largeInt, -reflectedObject.largeInt), false, false, false, testExpression, context);
	}
	SECTION("Pow_int3")
	{
		Expression<float> testExpression(&context, "pow(largeInt, largeInt)");
		doChecks((float)pow(reflectedObject.largeInt, reflectedObject.largeInt), false, false, false, testExpression, context);
	}
	SECTION("Pow_bool")
	{
		Expression<float> testExpression(&context, "pow(aBoolean, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_stringConst")
	{
		Expression<float> testExpression(&context, "pow(\"10\", 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_string")
	{
		Expression<float> testExpression(&context, "pow(numberString, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_string2")
	{
		Expression<float> testExpression(&context, "pow(text, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_noarg")
	{
		Expression<float> testExpression(&context, "pow()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_morearg")
	{
		Expression<float> testExpression(&context, "pow(theInt, aFloat, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Pow_obj")
	{
		Expression<float> testExpression(&context, "pow(nestedObject, 10)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Ceil", "[builtins][ceil]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Ceil", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Ceil_Constant")
	{
		Expression<float> testExpression(&context, "ceil(42.1f)");
		doChecks<float>(ceil(42.1f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_Negative Constant")
	{
		Expression<float> testExpression(&context, "ceil(-42.1f)");
		doChecks<float>(ceil(-42.1f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_IntConstant")
	{
		Expression<float> testExpression(&context, "ceil(3)");
		doChecks<float>(ceil(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_Negative IntConstant")
	{
		Expression<float> testExpression(&context, "ceil(-3)");
		doChecks<float>(ceil(-3.0f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_Zero Variable")
	{
		Expression<float> testExpression(&context, "ceil(zeroFloat)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Ceil_Variable")
	{
		Expression<float> testExpression(&context, "ceil(aFloat)");
		doChecks<float>(ceil(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Ceil_Negative Variable")
	{
		Expression<float> testExpression(&context, "ceil(-aFloat)");
		doChecks<float>(ceil(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Ceil_IntVariable")
	{
		Expression<float> testExpression(&context, "ceil(theInt)");
		doChecks((float)ceil(reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Ceil_Negative IntVariable")
	{
		Expression<float> testExpression(&context, "ceil(-theInt)");
		doChecks((float)ceil(-reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Ceil_string")
	{
		Expression<float> testExpression(&context, "ceil(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Ceil_noarg")
	{
		Expression<float> testExpression(&context, "ceil()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Ceil_morearg")
	{
		Expression<float> testExpression(&context, "ceil(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Ceil_obj")
	{
		Expression<float> testExpression(&context, "ceil(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Floor", "[builtins][floor]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Floor", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Floor_Constant")
	{
		Expression<float> testExpression(&context, "floor(42.1f)");
		doChecks<float>(floor(42.1f), false, true, false, testExpression, context);
	}
	SECTION("Floor_Negative Constant")
	{
		Expression<float> testExpression(&context, "floor(-42.1f)");
		doChecks<float>(floor(-42.1f), false, true, false, testExpression, context);
	}
	SECTION("Floor_IntConstant")
	{
		Expression<float> testExpression(&context, "floor(3)");
		doChecks<float>(floor(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Floor_Negative IntConstant")
	{
		Expression<float> testExpression(&context, "floor(-3)");
		doChecks<float>(floor(-3.0f), false, true, false, testExpression, context);
	}
	SECTION("Floor_Zero Variable")
	{
		Expression<float> testExpression(&context, "floor(zeroFloat)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Floor_Variable")
	{
		Expression<float> testExpression(&context, "floor(aFloat)");
		doChecks<float>(floor(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Floor_Negative Variable")
	{
		Expression<float> testExpression(&context, "floor(-aFloat)");
		doChecks<float>(floor(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Floor_IntVariable")
	{
		Expression<float> testExpression(&context, "floor(theInt)");
		doChecks((float)floor(reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Floor_Negative IntVariable")
	{
		Expression<float> testExpression(&context, "floor(-theInt)");
		doChecks((float)floor(-reflectedObject.theInt), false, false, false, testExpression, context);
	}
	SECTION("Floor_string")
	{
		Expression<float> testExpression(&context, "floor(numberString)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Floor_noarg")
	{
		Expression<float> testExpression(&context, "floor()");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Floor_morearg")
	{
		Expression<float> testExpression(&context, "floor(theInt, aFloat)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
	SECTION("Floor_obj")
	{
		Expression<float> testExpression(&context, "floor(nestedObject)");
		doChecks(0.0f, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: FindInString", "[builtins][findInString]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_FindInString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("FindInString_cc1")
	{
		Expression<int> testExpression(&context, "findInString(11.1f, 1)");
		doChecks(0, false, true, false, testExpression, context);
	}
	SECTION("FindInString_cc2")
	{
		Expression<int> testExpression(&context, "findInString(0, 11.0f)");
		doChecks(-1, false, true, false, testExpression, context);
	}
	SECTION("FindInString_cc3")
	{
		Expression<int> testExpression(&context, "findInString(9, -9)");
		doChecks(-1, false, true, false, testExpression, context);
	}
	SECTION("FindInString_cc4")
	{
		Expression<int> testExpression(&context, "findInString(\"test\", \"t\")");
		doChecks(0, false, true, false, testExpression, context);
	}
	SECTION("FindInString_cc5")
	{
		Expression<int> testExpression(&context, "findInString(\"test\", \"est\")");
		doChecks(1, false, true, false, testExpression, context);
	}
	SECTION("FindInString_cc6")
	{
		Expression<int> testExpression(&context, "findInString(\"test\", \"xxx\")");
		doChecks(-1, false, true, false, testExpression, context);
	}
	SECTION("FindInString_float1")
	{
		Expression<int> testExpression(&context, "findInString(aFloat, 9)");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("FindInString_float2")
	{
		Expression<int> testExpression(&context, "findInString(aFloat, 1000.0f)");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("FindInString_int1")
	{
		Expression<int> testExpression(&context, "findInString(largeInt, 7)");
		doChecks(6, false, false, false, testExpression, context);
	}
	SECTION("FindInString_int2")
	{
		Expression<int> testExpression(&context, "findInString(largeInt, -largeInt)");
		doChecks(-1, false, false, false, testExpression, context);
	}
	SECTION("FindInString_int3")
	{
		Expression<int> testExpression(&context, "findInString(largeInt, largeInt)");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("FindInString_bool")
	{
		Expression<int> testExpression(&context, "findInString(aBoolean, \"1\")");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("FindInString_stringConst")
	{
		Expression<int> testExpression(&context, "findInString(\"10\", 10)");
		doChecks(0, false, true, false, testExpression, context);
	}
	SECTION("FindInString_string")
	{
		Expression<int> testExpression(&context, "findInString(numberString, \".4\")");
		doChecks(3, false, false, false, testExpression, context);
	}
	SECTION("FindInString_string2")
	{
		Expression<int> testExpression(&context, "findInString(text, \"ll\")");
		doChecks(2, false, false, false, testExpression, context);
	}
	SECTION("FindInString_noarg")
	{
		Expression<int> testExpression(&context, "findInString()");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("FindInString_morearg")
	{
		Expression<int> testExpression(&context, "findInString(theInt, aFloat, 10)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("FindInString_obj")
	{
		Expression<int> testExpression(&context, "findInString(nestedObject, 10)");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: ReplaceInString", "[builtins][replaceInString]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_ReplaceInString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("ReplaceInString_cc1")
	{
		Expression<std::string> testExpression(&context, "replaceInString(11.1f, 1, 2)");
		doChecks(std::string("22.2"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_cc2")
	{
		Expression<std::string> testExpression(&context, "replaceInString(0, 11.0f, 12)");
		doChecks(std::string("0"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_cc3")
	{
		Expression<std::string> testExpression(&context, "replaceInString(\"9\", 9, \"7\")");
		doChecks(std::string("7"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_cc4")
	{
		Expression<std::string> testExpression(&context, "replaceInString(\"test\", \"t\", \"tt\")");
		doChecks(std::string("ttestt"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_cc5")
	{
		Expression<std::string> testExpression(&context, "replaceInString(\"test\", \"est\", \"om\")");
		doChecks(std::string("tom"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_cc6")
	{
		Expression<std::string> testExpression(&context, "replaceInString(\"test\", \"xxx\", false)");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_float1")
	{
		Expression<std::string> testExpression(&context, "replaceInString(aFloat, 9, true)");
		doChecks(std::string("111.1"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_float2")
	{
		Expression<std::string> testExpression(&context, "replaceInString(aFloat, 1000.0f, \"test\")");
		doChecks(std::string("999.9"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_int1")
	{
		Expression<std::string> testExpression(&context, "replaceInString(largeInt, 7, 789)");
		doChecks(std::string("123456789"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_int2")
	{
		Expression<std::string> testExpression(&context, "replaceInString(largeInt, -largeInt, 0)");
		doChecks(std::string("1234567"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_int3")
	{
		Expression<std::string> testExpression(&context, "replaceInString(largeInt, largeInt, largeInt)");
		doChecks(std::string("1234567"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_bool")
	{
		Expression<std::string> testExpression(&context, "replaceInString(aBoolean, \"1\", false)");
		doChecks(std::string("0"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_stringConst")
	{
		Expression<std::string> testExpression(&context, "replaceInString(\"10\", 10, \"42\")");
		doChecks(std::string("42"), false, true, false, testExpression, context);
	}
	SECTION("ReplaceInString_string")
	{
		Expression<std::string> testExpression(&context, "replaceInString(numberString, \".4\", 4)");
		doChecks(std::string("1234"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_string2")
	{
		Expression<std::string> testExpression(&context, "replaceInString(text, \"ll\", text)");
		doChecks(std::string("HeHello!o!"), false, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_noarg")
	{
		Expression<std::string> testExpression(&context, "replaceInString()");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_morearg")
	{
		Expression<std::string> testExpression(&context, "replaceInString(theInt, aFloat, 10, false)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("ReplaceInString_obj")
	{
		Expression<std::string> testExpression(&context, "replaceInString(nestedObject, 10, false)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: StringLength", "[builtins][stringLength]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_StringLength", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("StringLength_cc1")
	{
		Expression<int> testExpression(&context, "stringLength(11.1f)");
		doChecks(4, false, true, false, testExpression, context);
	}
	SECTION("StringLength_cc2")
	{
		Expression<int> testExpression(&context, "stringLength(0)");
		doChecks(1, false, true, false, testExpression, context);
	}
	SECTION("StringLength_cc3")
	{
		Expression<int> testExpression(&context, "stringLength(9)");
		doChecks(1, false, true, false, testExpression, context);
	}
	SECTION("StringLength_cc4")
	{
		Expression<int> testExpression(&context, "stringLength(\"test\")");
		doChecks(4, false, true, false, testExpression, context);
	}
	SECTION("StringLength_cc5")
	{
		Expression<int> testExpression(&context, "stringLength(\"\")");
		doChecks(0, false, true, false, testExpression, context);
	}
	SECTION("StringLength_float1")
	{
		Expression<int> testExpression(&context, "stringLength(aFloat)");
		doChecks(5, false, false, false, testExpression, context);
	}
	SECTION("StringLength_float2")
	{
		Expression<int> testExpression(&context, "stringLength(zeroFloat)");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("StringLength_int1")
	{
		Expression<int> testExpression(&context, "stringLength(largeInt)");
		doChecks(7, false, false, false, testExpression, context);
	}
	SECTION("StringLength_int2")
	{
		Expression<int> testExpression(&context, "stringLength(-largeInt)");
		doChecks(8, false, false, false, testExpression, context);
	}
	SECTION("StringLength_bool")
	{
		Expression<int> testExpression(&context, "stringLength(aBoolean)");
		doChecks(1, false, false, false, testExpression, context);
	}
	SECTION("StringLength_stringConst")
	{
		Expression<int> testExpression(&context, "stringLength(\"10\")");
		doChecks(2, false, true, false, testExpression, context);
	}
	SECTION("StringLength_string")
	{
		Expression<int> testExpression(&context, "stringLength(numberString)");
		doChecks(5, false, false, false, testExpression, context);
	}
	SECTION("StringLength_string2")
	{
		Expression<int> testExpression(&context, "stringLength(text)");
		doChecks(6, false, false, false, testExpression, context);
	}
	SECTION("StringLength_noarg")
	{
		Expression<int> testExpression(&context, "stringLength()");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("StringLength_morearg")
	{
		Expression<int> testExpression(&context, "stringLength(theInt, aFloat)");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("StringLength_obj")
	{
		Expression<int> testExpression(&context, "stringLength(nestedObject)");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: SubString", "[builtins][subString]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_SubString", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("SubString_cc1")
	{
		Expression<std::string> testExpression(&context, "subString(11.1f, 1, 3)");
		doChecks(std::string("1.1"), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc2")
	{
		Expression<std::string> testExpression(&context, "subString(0, 11.0f, 12)");
		doChecks(std::string(""), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc3")
	{
		Expression<std::string> testExpression(&context, "subString(\"9\", 9, 0)");
		doChecks(std::string(""), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc4")
	{
		Expression<std::string> testExpression(&context, "subString(\"test\", 0, 4)");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc5")
	{
		Expression<std::string> testExpression(&context, "subString(\"test\", 0, 100)");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc6")
	{
		Expression<std::string> testExpression(&context, "subString(\"test\", 3, 1)");
		doChecks(std::string("t"), false, true, false, testExpression, context);
	}
	SECTION("SubString_float1")
	{
		Expression<std::string> testExpression(&context, "subString(aFloat, 4, 5)");
		doChecks(std::string("9"), false, false, false, testExpression, context);
	}
	SECTION("SubString_float2")
	{
		Expression<std::string> testExpression(&context, "subString(aFloat, 1000.0f, 1.1f)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("SubString_int1")
	{
		Expression<std::string> testExpression(&context, "subString(largeInt, -7, 789)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("SubString_int2")
	{
		Expression<std::string> testExpression(&context, "subString(largeInt, -largeInt, 0)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("SubString_int3")
	{
		Expression<std::string> testExpression(&context, "subString(largeInt, largeInt, largeInt)");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("SubString_bool")
	{
		Expression<std::string> testExpression(&context, "subString(aBoolean, 0, 1)");
		doChecks(std::string("1"), false, false, false, testExpression, context);
	}
	SECTION("SubString_stringConst")
	{
		Expression<std::string> testExpression(&context, "subString(\"10\", 1, 1)");
		doChecks(std::string("0"), false, true, false, testExpression, context);
	}
	SECTION("SubString_string")
	{
		Expression<std::string> testExpression(&context, "subString(numberString, 3, 2)");
		doChecks(std::string(".4"), false, false, false, testExpression, context);
	}
	SECTION("SubString_string2")
	{
		Expression<std::string> testExpression(&context, "subString(text, 2, 2)");
		doChecks(std::string("ll"), false, false, false, testExpression, context);
	}
	SECTION("SubString_noarg")
	{
		Expression<std::string> testExpression(&context, "subString()");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("SubString_morearg")
	{
		Expression<std::string> testExpression(&context, "subString(theInt, aFloat, 10, false)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("SubString_obj")
	{
		Expression<std::string> testExpression(&context, "subString(nestedObject, 10, 11)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
}


TEST_CASE("Builtin functions test: Select", "[builtins][select]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Select", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Select_cc1")
	{
		Expression<int> testExpression(&context, "select(true, 1, 3)");
		doChecks(1, false, true, false, testExpression, context);
	}
	SECTION("Select_cc2")
	{
		Expression<float> testExpression(&context, "select(false, 11.0f, 12)");
		doChecks(12.0f, false, true, false, testExpression, context);
	}
	SECTION("Select_cc3")
	{
		Expression<std::string> testExpression(&context, "select(true, \"test\", \"bla\")");
		doChecks(std::string("test"), false, true, false, testExpression, context);
	}
	SECTION("Select_cc4")
	{
		Expression<bool> testExpression(&context, "select(false, false, true)");
		doChecks(true, false, true, false, testExpression, context);
	}
	SECTION("Select_float1")
	{
		Expression<int> testExpression(&context, "select(aFloat > 900, 4, 5)");
		doChecks(4, false, false, false, testExpression, context);
	}
	SECTION("Select_float2")
	{
		Expression<std::string> testExpression(&context, "select(aFloat != 0, \"test\", 1.0f)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("Select_int1")
	{
		Expression<int> testExpression(&context, "select(largeInt == 1234567, -7, 789)");
		doChecks(-7, false, false, false, testExpression, context);
	}
	SECTION("Select_int2")
	{
		Expression<int> testExpression(&context, "select(aBoolean, -largeInt, largeInt)");
		doChecks(-1234567, false, false, false, testExpression, context);
	}
	SECTION("Select_int3")
	{
		Expression<int> testExpression(&context, "select(largeInt != 0, largeInt, largeInt)");
		doChecks(1234567, false, false, false, testExpression, context);
	}
	SECTION("Select_bool")
	{
		Expression<std::string> testExpression(&context, "select(!aBoolean, \"0\", \"1\")");
		doChecks(std::string("1"), false, false, false, testExpression, context);
	}
	SECTION("Select_string1")
	{
		Expression<std::string> testExpression(&context, "select(stringLength(\"10\") > 0, text, numberString)");
		doChecks(std::string("Hello!"), false, false, false, testExpression, context);
	}
	SECTION("Select_string2")
	{
		Expression<float> testExpression(&context, "select(numberString == \"\", 3.0f, 2.0f)");
		doChecks(2.0f, false, false, false, testExpression, context);
	}
	SECTION("Select_string3")
	{
		Expression<std::string> testExpression(&context, "select(text == numberString, text, numberString)");
		doChecks(std::string("123.4"), false, false, false, testExpression, context);
	}
	SECTION("Select_noarg")
	{
		Expression<std::string> testExpression(&context, "select()");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("Select_morearg")
	{
		Expression<std::string> testExpression(&context, "select(aBoolean, aFloat, 10, largeInt)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
	SECTION("Select_obj")
	{
		Expression<std::string> testExpression(&context, "select(nestedObject, 10, 11)");
		doChecks(std::string(""), true, false, false, testExpression, context);
	}
}


TEST_CASE("Indirection tests", "[indirection]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("indirection", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("NestedSelfObject String")
	{
		Expression<std::string> testExpression(&context, "nestedSelfObject.numberString");
		doChecks(std::string("123.4"), false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Int")
	{
		Expression<int> testExpression(&context, "nestedSelfObject.theInt");
		doChecks(42, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Boolean")
	{
		Expression<bool> testExpression(&context, "nestedSelfObject.aBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Float")
	{
		Expression<float> testExpression(&context, "nestedSelfObject.aFloat");
		doChecks(999.9f, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Object")
	{
		Expression<ReflectedObject*> testExpression(&context, "nestedSelfObject.nestedSelfObject");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Vector")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nestedSelfObject.reflectableObjectsVector[0]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Map")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nestedSelfObject.reflectableObjectsMap[\"one\"]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("NestedSelfObject Error")
	{
		Expression<int> testExpression(&context, "nestedSelfObject.notAnInt");
		doChecks(0, true, false, false, testExpression, context);
	}

	SECTION("nestedObject String")
	{
		Expression<std::string> testExpression(&context, "nestedObject.someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("nestedObject Int")
	{
		Expression<int> testExpression(&context, "nestedObject.someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("nestedObject Boolean")
	{
		Expression<bool> testExpression(&context, "nestedObject.someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("nestedObject Float")
	{
		Expression<float> testExpression(&context, "nestedObject.someFloat");
		doChecks(1.1f, false, false, false, testExpression, context);
	}
	SECTION("nestedObject Object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nestedObject.nullObject");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nestedObject Error")
	{
		Expression<int> testExpression(&context, "nestedObject.notAnInt");
		doChecks(0, true, false, false, testExpression, context);
	}

	SECTION("nestedObjectPointer String")
	{
		Expression<std::string> testExpression(&context, "nestedObjectPointer.someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Int")
	{
		Expression<int> testExpression(&context, "nestedObjectPointer.someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Boolean")
	{
		Expression<bool> testExpression(&context, "nestedObjectPointer.someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Float")
	{
		Expression<float> testExpression(&context, "nestedObjectPointer.someFloat");
		doChecks(1.1f, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nestedObjectPointer.nullObject");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectPointer Error")
	{
		Expression<int> testExpression(&context, "nestedObjectPointer.notAnInt");
		doChecks(0, true, false, false, testExpression, context);
	}

	SECTION("nestedObjectUniquePointer String")
	{
		Expression<std::string> testExpression(&context, "nestedObjectUniquePointer.someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Int")
	{
		Expression<int> testExpression(&context, "nestedObjectUniquePointer.someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Boolean")
	{
		Expression<bool> testExpression(&context, "nestedObjectUniquePointer.someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Float")
	{
		Expression<float> testExpression(&context, "nestedObjectUniquePointer.someFloat");
		doChecks(1.1f, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nestedObjectUniquePointer.nullObject");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nestedObjectUniquePointer Error")
	{
		Expression<int> testExpression(&context, "nestedObjectUniquePointer.notAnInt");
		doChecks(0, true, false, false, testExpression, context);
	}

	SECTION("nested null object String")
	{
		Expression<std::string> testExpression(&context, "nullObject.text");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("nested null object Int")
	{
		Expression<int> testExpression(&context, "nullObject.theInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("nested null object Boolean")
	{
		Expression<bool> testExpression(&context, "nullObject.no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("nested null object Float")
	{
		Expression<float> testExpression(&context, "nullObject.aFloat");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("nested null object ObjectPtr")
	{
		Expression<ReflectedObject*> testExpression(&context, "nullObject.nullObject");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nested null object Object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.nestedObject");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nested null object UniquePtr")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.nestedObjectUniquePointer");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("nested null object Error")
	{
		Expression<int> testExpression(&context, "nullObject.notAnInt");
		doChecks(0, true, false, false, testExpression, context);
	}

	SECTION("badBaseObject Error")
	{
		Expression<int> testExpression(&context, "notAPointer.theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing base object Error")
	{
		Expression<int> testExpression(&context, ".theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("null Object")
	{
		Expression<int> testExpression(&context, "nestedSelfObject.nestedSelfObject.theInt");
		doChecks(0, false, false, false, testExpression, context);
	}
}


TEST_CASE("Assign tests", "[assign]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("Assign", &errorManager);
	context.addScope(&reflectedObject, true);

	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	const char* customTypeName = "MyType";
	TypeRegistry::get()->removeType(customTypeName);
	std::unique_ptr<CustomTypeInfo> customType(new CustomTypeInfo(customTypeName));
	TypeRegistry::get()->registerType(customTypeName, customType.get());
	customType->addFloatMember("myFloat", 0.001f);
	customType->addIntMember("myInt", 54321);
	customType->addStringMember("myString", "foo");
	customType->addBoolMember("myBoolean", true);
	customType->addObjectMember("myObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject", &reflectedObject, objectTypeInfo);
	std::unique_ptr<CustomTypeInstance> typeInstance(customType->createInstance());
	context.addCustomTypeScope(customType.get(), typeInstance.get());

	const char* customStaticTypeName = "MyStaticType";
	TypeRegistry::get()->removeType(customStaticTypeName);
	std::unique_ptr<CustomTypeInfo> customStaticType(new CustomTypeInfo(customStaticTypeName));
	customStaticType->addObjectMember("myStaticObject", &reflectedObject, objectTypeInfo);
	customStaticType->addObjectMember("myStaticCustomObject", typeInstance.get(), customType.get());
	std::unique_ptr<CustomTypeInstance> staticTypeInstance(customStaticType->createInstance());
	context.addCustomTypeScope(customStaticType.get(), staticTypeInstance.get(), true);

	SECTION("Assign reflected int")
	{
		Expression<void> testExpression(&context, "theInt = -99");
		checkAssignment(reflectedObject.theInt, -99, false, false, false, testExpression, context);
	}
	SECTION("Assign reflected float")
	{
		Expression<void> testExpression(&context, "aFloat = 11.0f");
		checkAssignment(reflectedObject.aFloat, 11.0f, false, false, false, testExpression, context);
	}
	SECTION("Assign reflected bool")
	{
		Expression<void> testExpression(&context, "aBoolean = no");
		checkAssignment(reflectedObject.aBoolean, false, false, false, false, testExpression, context);
	}
	SECTION("Assign reflected string")
	{
		Expression<void> testExpression(&context, "text = \"World!\"");
		checkAssignment(reflectedObject.text, std::string("World!"), false, false, false, testExpression, context);
	}
	SECTION("Assign reflected object")
	{
		Expression<void> testExpression(&context, "nestedObjectPointer = nestedObject");
		checkAssignment((Reflectable*&)reflectedObject.nestedObjectPointer, (Reflectable*)&reflectedObject.nestedObject, false, false, false, testExpression, context);	
	}
	SECTION("Assign reflected object 2")
	{
		Expression<void> testExpression(&context, "nestedSelfObject.nestedObjectPointer = nestedObject");
		checkAssignment((Reflectable * &)reflectedObject.nestedSelfObject->nestedObjectPointer, (Reflectable*)& reflectedObject.nestedObject, false, false, false, testExpression, context);
	}
	SECTION("Assign custom object")
	{
		Expression<void> testExpression(&context, "myNullObject = nestedSelfObject");
		doCommonChecks(&testExpression, false, false, false, context);
		testExpression.getValue(&context);
		testExpression.getInterpretedValue(&context);
	}
	SECTION("Assign static custom object")
	{
		Expression<void> testExpression(&context, "myStaticCustomObject.myNullObject = nestedSelfObject");
		doCommonChecks(&testExpression, false, false, false, context);
		testExpression.getValue(&context);
		testExpression.getInterpretedValue(&context);
	}
	SECTION("Assign nonWritable int")
	{
		Expression<void> testExpression(&context, "largeInt = -99");
		checkAssignment(reflectedObject.theInt, -99, true, false, false, testExpression, context);
	}
	SECTION("Assign nonWritable reflected float")
	{
		Expression<void> testExpression(&context, "zeroFloat = 11.0f");
		checkAssignment(reflectedObject.zeroFloat, 11.0f, true, false, false, testExpression, context);	
	}
	SECTION("Assign nonWritable reflected bool")
	{
		Expression<void> testExpression(&context, "no = true");
		checkAssignment(reflectedObject.no, true, true, false, false, testExpression, context);		
	}
	SECTION("Assign nonWritable reflected string")
	{
		Expression<void> testExpression(&context, "numberString = \"456.7\"");
		checkAssignment(reflectedObject.numberString, std::string("456.7"), true, false, false, testExpression, context);		
	}
	SECTION("Assign nonWritable reflected object")
	{
		Expression<void> testExpression(&context, "nestedObjectUniquePointer = nestedObject");
		checkAssignment((Reflectable*&)reflectedObject.nestedObjectPointer, (Reflectable*)&reflectedObject.nestedObject, true, false, false, testExpression, context);	
	}

	SECTION("Assign custom int")
	{
		Expression<void> testExpression(&context, "myInt = -99");
		checkAssignmentCustom(typeInstance.get(), "myInt", -99, false, false, false, testExpression, context);
	}
	SECTION("Assign custom float")
	{
		Expression<void> testExpression(&context, "myFloat = 11.0f");
		checkAssignmentCustom(typeInstance.get(), "myFloat", 11.0f, false, false, false, testExpression, context);	
	}
	SECTION("Assign custom bool")
	{
		Expression<void> testExpression(&context, "myBoolean = false");
		checkAssignmentCustom(typeInstance.get(), "myBoolean", false, false, false, false, testExpression, context);		
	}
	SECTION("Assign custom string")
	{
		Expression<void> testExpression(&context, "myString = \"bar\"");
		checkAssignmentCustom(typeInstance.get(), "myString", std::string("bar"), false, false, false, testExpression, context);		
	}
	SECTION("Assign custom object")
	{
		Expression<void> testExpression(&context, "myObject = nestedSelfObject");
		checkAssignmentCustom(typeInstance.get(), "myObject", (Reflectable*)reflectedObject.nestedSelfObject, false, false, false, testExpression, context);		
	}
}




TEST_CASE("Expression assign tests", "[assign][expressionassign]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("Assign", &errorManager);
	context.addScope(&reflectedObject, true);

	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	const char* customTypeName = "MyType";
	TypeRegistry::get()->removeType(customTypeName);
	std::unique_ptr<CustomTypeInfo> customType(new CustomTypeInfo(customTypeName));
	TypeRegistry::get()->registerType(customTypeName, customType.get());
	customType->addFloatMember("myFloat", 0.001f);
	customType->addIntMember("myInt", 54321);
	customType->addStringMember("myString", "foo");
	customType->addBoolMember("myBoolean", true);
	customType->addObjectMember("myObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject", &reflectedObject, objectTypeInfo);
	std::unique_ptr<CustomTypeInstance> typeInstance(customType->createInstance());
	context.addCustomTypeScope(customType.get(), typeInstance.get());


	SECTION("Assign reflected int")
	{
		ExpressionAssignment<int> testExpression(&context, "theInt");
		checkAssignExpression(reflectedObject.theInt, -99, false, testExpression, context);
	}
	SECTION("Assign reflected float")
	{
		ExpressionAssignment<float> testExpression(&context, "aFloat");
		checkAssignExpression(reflectedObject.aFloat, 11.0f, false, testExpression, context);
	}
	SECTION("Assign reflected bool")
	{
		ExpressionAssignment<bool> testExpression(&context, "aBoolean");
		checkAssignExpression(reflectedObject.aBoolean, false, false, testExpression, context);
	}
	SECTION("Assign reflected string")
	{
		ExpressionAssignment<std::string> testExpression(&context, "text");
		checkAssignExpression(reflectedObject.text, std::string("World!"), false, testExpression, context);
	}
	SECTION("Assign reflected object")
	{
		ExpressionAssignment<NestedReflectedObject*> testExpression(&context, "nestedObjectPointer");
		checkAssignExpression(reflectedObject.nestedObjectPointer, &reflectedObject.nestedObject, false, testExpression, context);	
	}

	SECTION("Assign nonWritable int")
	{
		ExpressionAssignment<int> testExpression(&context, "largeInt");
		checkAssignExpression(reflectedObject.theInt, -99, true, testExpression, context);
	}
	SECTION("Assign nonWritable reflected float")
	{
		ExpressionAssignment<float> testExpression(&context, "zeroFloat");
		checkAssignExpression(reflectedObject.zeroFloat, 11.0f, true, testExpression, context);	
	}
	SECTION("Assign nonWritable reflected bool")
	{
		ExpressionAssignment<bool> testExpression(&context, "no");
		checkAssignExpression(reflectedObject.no, true, true, testExpression, context);		
	}
	SECTION("Assign nonWritable reflected string")
	{
		ExpressionAssignment<std::string> testExpression(&context, "numberString");
		checkAssignExpression(reflectedObject.numberString, std::string("456.7"), true, testExpression, context);		
	}
	SECTION("Assign nonWritable reflected object")
	{
		ExpressionAssignment<NestedReflectedObject*> testExpression(&context, "nestedObjectUniquePointer");
		checkAssignExpression(reflectedObject.nestedObjectPointer, &reflectedObject.nestedObject, true, testExpression, context);	
	}

	SECTION("Assign custom int")
	{
		ExpressionAssignment<int> testExpression(&context, "myInt");
		checkAssignExpressionCustom(typeInstance.get(), "myInt", -99, false, testExpression, context);
	}
	SECTION("Assign custom float")
	{
		ExpressionAssignment<float> testExpression(&context, "myFloat");
		checkAssignExpressionCustom(typeInstance.get(), "myFloat", 11.0f, false, testExpression, context);	
	}
	SECTION("Assign custom bool")
	{
		ExpressionAssignment<bool> testExpression(&context, "myBoolean");
		checkAssignExpressionCustom(typeInstance.get(), "myBoolean", false, false, testExpression, context);		
	}
	SECTION("Assign custom string")
	{
		ExpressionAssignment<std::string> testExpression(&context, "myString");
		checkAssignExpressionCustom(typeInstance.get(), "myString", std::string("bar"), false, testExpression, context);		
	}
	SECTION("Assign custom object")
	{
		ExpressionAssignment<ReflectedObject*> testExpression(&context, "myObject");
		checkAssignExpressionCustom(typeInstance.get(), "myObject", reflectedObject.nestedSelfObject, false, testExpression, context);		
	}
}

TEST_CASE("Expression any assign tests", "[assign][expressionassign]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("Assign", &errorManager);
	context.addScope(&reflectedObject, true);

	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	const char* customTypeName = "MyType";
	TypeRegistry::get()->removeType(customTypeName);
	std::unique_ptr<CustomTypeInfo> customType(new CustomTypeInfo(customTypeName));
	TypeRegistry::get()->registerType(customTypeName, customType.get());
	customType->addFloatMember("myFloat", 0.001f);
	customType->addIntMember("myInt", 54321);
	customType->addStringMember("myString", "foo");
	customType->addBoolMember("myBoolean", true);
	customType->addObjectMember("myObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject", &reflectedObject, objectTypeInfo);
	std::unique_ptr<CustomTypeInstance> typeInstance(customType->createInstance());
	context.addCustomTypeScope(customType.get(), typeInstance.get());


	SECTION("Assign reflected int")
	{
		ExpressionAssignAny testExpression(&context, "theInt");
		checkAnyAssignExpression(reflectedObject.theInt, -99, false, testExpression, context);
	}
	SECTION("Assign reflected float")
	{
		ExpressionAssignAny testExpression(&context, "aFloat");
		checkAnyAssignExpression(reflectedObject.aFloat, 11.0f, false, testExpression, context);
	}
	SECTION("Assign reflected bool")
	{
		ExpressionAssignAny testExpression(&context, "aBoolean");
		checkAnyAssignExpression(reflectedObject.aBoolean, false, false, testExpression, context);
	}
	SECTION("Assign reflected string")
	{
		ExpressionAssignAny testExpression(&context, "text");
		checkAnyAssignExpression(reflectedObject.text, std::string("World!"), false, testExpression, context);
	}
	SECTION("Assign reflected object")
	{
		ExpressionAssignAny testExpression(&context, "nestedObjectPointer");
		checkAnyAssignExpression(reflectedObject.nestedObjectPointer, &reflectedObject.nestedObject, false, testExpression, context);
	}

	SECTION("Assign nonWritable int")
	{
		ExpressionAssignAny testExpression(&context, "largeInt");
		checkAnyAssignExpression(reflectedObject.theInt, -99, true, testExpression, context);
	}
	SECTION("Assign nonWritable reflected float")
	{
		ExpressionAssignAny testExpression(&context, "zeroFloat");
		checkAnyAssignExpression(reflectedObject.zeroFloat, 11.0f, true, testExpression, context);
	}
	SECTION("Assign nonWritable reflected bool")
	{
		ExpressionAssignAny testExpression(&context, "no");
		checkAnyAssignExpression(reflectedObject.no, true, true, testExpression, context);
	}
	SECTION("Assign nonWritable reflected string")
	{
		ExpressionAssignAny testExpression(&context, "numberString");
		checkAnyAssignExpression(reflectedObject.numberString, std::string("456.7"), true, testExpression, context);
	}
	SECTION("Assign nonWritable reflected object")
	{
		ExpressionAssignAny testExpression(&context, "nestedObjectUniquePointer");
		checkAnyAssignExpression(reflectedObject.nestedObjectPointer, &reflectedObject.nestedObject, true, testExpression, context);
	}

	SECTION("Assign custom int")
	{
		ExpressionAssignAny testExpression(&context, "myInt");
		checkAnyAssignExpressionCustom(typeInstance.get(), "myInt", -99, false, testExpression, context);
	}
	SECTION("Assign custom float")
	{
		ExpressionAssignAny testExpression(&context, "myFloat");
		checkAnyAssignExpressionCustom(typeInstance.get(), "myFloat", 11.0f, false, testExpression, context);
	}
	SECTION("Assign custom bool")
	{
		ExpressionAssignAny testExpression(&context, "myBoolean");
		checkAnyAssignExpressionCustom(typeInstance.get(), "myBoolean", false, false, testExpression, context);
	}
	SECTION("Assign custom string")
	{
		ExpressionAssignAny testExpression(&context, "myString");
		checkAnyAssignExpressionCustom(typeInstance.get(), "myString", std::string("bar"), false, testExpression, context);
	}
	SECTION("Assign custom object")
	{
		ExpressionAssignAny testExpression(&context, "myObject");
		checkAnyAssignExpressionCustom(typeInstance.get(), "myObject", reflectedObject.nestedSelfObject, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Vector", "[containers][vector]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("vectorContainer", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Vector get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsVector[0]");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
	}
	SECTION("Vector get non-pointer object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "objectVector[0]");
		doChecks(&reflectedObject.objectVector[0], false, false, false, testExpression, context);
	}
	SECTION("Vector get non-pointer object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "objectVector[-1]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Vector get float")
	{
		Expression<float> testExpression(&context, "floatVector[0]");
		doChecks(reflectedObject.floatVector[0], false, false, false, testExpression, context);
	}
	SECTION("Vector get float out of range")
	{
		Expression<float> testExpression(&context, "floatVector[-1]");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Vector get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsVector[5000]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Vector get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsVector[-1]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Vector get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsVector[0].someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Vector get int object")
	{
		Expression<int> testExpression(&context, "reflectableObjectsVector[0].someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Vector get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableObjectsVector[0].someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Vector out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsVector[10].someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Vector out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsVector[theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Vector out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsVector[-theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null vector get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsVector[0]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Bad vector name error")
	{
		Expression<int> testExpression(&context, "badVectorName[0]");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing vector name error")
	{
		Expression<int> testExpression(&context, "[0].theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Vector of unique_ptr", "[containers][vector]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("vectorContainer", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Vector get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsVector[0]");
		doChecks(reflectedObject.reflectableUniqueObjectsVector[0].get(), false, false, false, testExpression, context);
	}
	SECTION("Vector get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsVector[5000]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Vector get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsVector[-1]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Vector get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableUniqueObjectsVector[0].someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Vector get int object")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsVector[0].someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Vector get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableUniqueObjectsVector[0].someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Vector out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableUniqueObjectsVector[10].someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Vector out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsVector[theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Vector out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsVector[-theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null vector get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsVector[0]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Bad vector name error")
	{
		Expression<int> testExpression(&context, "badVectorName[0]");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing vector name error")
	{
		Expression<int> testExpression(&context, "[0].theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Map", "[containers][map]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("mapContainer", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[0]");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
	}
	SECTION("Map get string")
	{
		Expression<std::string> testExpression(&context, "intToStringMap[1]");
		doChecks(reflectedObject.intToStringMap[1], false, false, false, testExpression, context);
	}
	SECTION("Map get string, not found")
	{
		Expression<std::string> testExpression(&context, "intToStringMap[0]");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[\"two\"]");
		doChecks(reflectedObject.nestedObjectUniquePointer.get(), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index uppercase")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[\"ONE\"]");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[5000]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index not found")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[\"OneHundred\"]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[-1]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsMap[0].someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Map get int object")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMap[0].someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Map get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableObjectsMap[0].someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Map out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsMap[10].someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Map out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMap[text].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Map out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMap[-theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsMap[0]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null map get object 2")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsMap[\"one\"]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Bad map name error")
	{
		Expression<int> testExpression(&context, "badMapName[\"two\"]");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing map name error")
	{
		Expression<int> testExpression(&context, "[\"one\"].theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Map with custom comparator", "[containers][map]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("mapContainer", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare[0]");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare[\"two\"]");
		doChecks(reflectedObject.nestedObjectUniquePointer.get(), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index uppercase")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare[\"ONE\"]");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare[5000]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index not found")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare[\"OneHundred\"]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMapCustomCompare[-1]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsMapCustomCompare[0].someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Map get int object")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMapCustomCompare[0].someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Map get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableObjectsMapCustomCompare[0].someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Map out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableObjectsMapCustomCompare[10].someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Map out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMapCustomCompare[text].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Map out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableObjectsMapCustomCompare[-theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsMap[0]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null map get object 2")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsMap[\"one\"]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Bad map name error")
	{
		Expression<int> testExpression(&context, "badMapName[\"two\"]");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing map name error")
	{
		Expression<int> testExpression(&context, "[\"one\"].theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Containers tests: Map of unique_ptr", "[containers][map]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("mapContainer", &errorManager);
	context.addScope(&reflectedObject, true);

	SECTION("Map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap[0]");
		doChecks(reflectedObject.reflectableUniqueObjectsMap.begin()->second.get(), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap[\"two\"]");
		doChecks(reflectedObject.reflectableUniqueObjectsMap["two"].get(), false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index uppercase")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap[\"ONE\"]");
		doChecks(reflectedObject.reflectableUniqueObjectsMap["one"].get(), false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap[5000]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object, string index not found")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap[\"OneHundred\"]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get object out of range, negative")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableUniqueObjectsMap[-1]");
		doChecks<NestedReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Map get string object")
	{
		Expression<std::string> testExpression(&context, "reflectableUniqueObjectsMap[0].someString");
		doChecks(std::string("test"), false, false, false, testExpression, context);
	}
	SECTION("Map get int object")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsMap[0].someInt");
		doChecks(21, false, false, false, testExpression, context);
	}
	SECTION("Map get boolean object")
	{
		Expression<bool> testExpression(&context, "reflectableUniqueObjectsMap[0].someBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Map out of range get string")
	{
		Expression<std::string> testExpression(&context, "reflectableUniqueObjectsMap[10].someString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Map out of range, variable index")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsMap[text].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Map out of range, negative variable index")
	{
		Expression<int> testExpression(&context, "reflectableUniqueObjectsMap[-theInt].someInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsMap[0]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null map get object 2")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "nullObject.reflectableObjectsMap[\"one\"]");
		doChecks((NestedReflectedObject*)nullptr, false, false, false, testExpression, context);
	}
	SECTION("Bad map name error")
	{
		Expression<int> testExpression(&context, "badMapName[\"two\"]");
		doChecks(0, true, false, false, testExpression, context);
	}
	SECTION("Missing map name error")
	{
		Expression<int> testExpression(&context, "[\"one\"].theInt");
		doChecks(0, true, false, false, testExpression, context);
	}
}


TEST_CASE("Member Functions", "[memberfunctions]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context("builtinTests_Select", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Get float")
	{
		Expression<float> testExpression(&context, "getFloat()");
		doChecks(reflectedObject.getFloat(), false, false, false, testExpression, context);
	}
	SECTION("Get int")
	{
		Expression<int> testExpression(&context, "getint()");
		doChecks(reflectedObject.getInt(), false, false, false, testExpression, context);
	}
	SECTION("Get bool")
	{
		Expression<bool> testExpression(&context, "getBoolean()");
		doChecks(reflectedObject.getBoolean(), false, false, false, testExpression, context);
	}
	SECTION("Get string")
	{
		Expression<std::string> testExpression(&context, "getString()");
		doChecks(reflectedObject.getString(), false, false, false, testExpression, context);
	}
	SECTION("Get object")
	{
		Expression<ReflectedObject*> testExpression(&context, "getObject()");
		doChecks(reflectedObject.getObject(), false, false, false, testExpression, context);
	}

	SECTION("Const get float")
	{
		Expression<float> testExpression(&context, "getConstantFloat()");
		doChecks(reflectedObject.getConstantFloat(), false, false, false, testExpression, context);
	}
	SECTION("Const get int")
	{
		Expression<int> testExpression(&context, "getConstInt()");
		doChecks(reflectedObject.getConstInt(), false, false, false, testExpression, context);
	}
	SECTION("Const get bool")
	{
		Expression<bool> testExpression(&context, "getConstBool()");
		doChecks(reflectedObject.getConstBool(), false, false, false, testExpression, context);
	}
	SECTION("Const get string")
	{
		Expression<std::string> testExpression(&context, "getConstString()");
		doChecks(reflectedObject.getConstString(), false, false, false, testExpression, context);
	}
	SECTION("Const get object")
	{
		Expression<ReflectedObject*> testExpression(&context, "getConstObject()");
		doChecks(reflectedObject.getConstObject(), false, false, false, testExpression, context);
	}
	SECTION("Void function")
	{
		Expression<void> testExpression(&context, "doSomething()");
		testExpression.getValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}
	SECTION("Const void function")
	{
		Expression<void> testExpression(&context, "doSomethingConst()");
		testExpression.getValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}
	SECTION("Void function 2")
	{
		Expression<void> testExpression(&context, "checkTheseValues(aBoolean, theInt, text, nestedSelfObject)");
		testExpression.getValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}
	SECTION("string function 1")
	{
		Expression<std::string> testExpression(&context, "returnThisString(text)");
		doChecks(reflectedObject.text, false, false, false, testExpression, context);
	}
	SECTION("string function 1")
	{
		Expression<std::string> testExpression(&context, "addToString(text, aFloat)");
		doChecks(Tools::append(reflectedObject.text, reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("object function 1")
	{
		Expression<ReflectedObject*> testExpression(&context, "getThisObject(nestedSelfObject)");
		doChecks(reflectedObject.nestedSelfObject, false, false, false, testExpression, context);
	}

	SECTION("object base float")
	{
		Expression<float> testExpression(&context, "nestedSelfObject.getFloat()");
		doChecks(reflectedObject.nestedSelfObject->getFloat(), false, false, false, testExpression, context);
	}
	SECTION("object base int")
	{
		Expression<int> testExpression(&context, "nestedSelfObject.getInt()");
		doChecks(reflectedObject.nestedSelfObject->getInt(), false, false, false, testExpression, context);
	}
	SECTION("object base boolean")
	{
		Expression<bool> testExpression(&context, "nestedSelfObject.getBoolean()");
		doChecks(reflectedObject.nestedSelfObject->getBoolean(), false, false, false, testExpression, context);
	}
	SECTION("object base string")
	{
		Expression<std::string> testExpression(&context, "nestedSelfObject.getString()");
		doChecks(reflectedObject.nestedSelfObject->getString(), false, false, false, testExpression, context);
	}
	SECTION("object base object")
	{
		Expression<ReflectedObject*> testExpression(&context, "nestedSelfObject.getObject()");
		doChecks<ReflectedObject*>(reflectedObject.nestedSelfObject->getObject(), false, false, false, testExpression, context);
	}
	SECTION("object base void")
	{
		Expression<void> testExpression(&context, "nestedSelfObject.doSomething()");
		testExpression.getValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}

	SECTION("null base float")
	{
		Expression<float> testExpression(&context, "nestedSelfObject.nestedSelfObject.getFloat()");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("null base int")
	{
		Expression<int> testExpression(&context, "nestedSelfObject.nestedSelfObject.getInt()");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("null base boolean")
	{
		Expression<bool> testExpression(&context, "nestedSelfObject.nestedSelfObject.getBoolean()");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("null base string")
	{
		Expression<std::string> testExpression(&context, "nestedSelfObject.nestedSelfObject.getString()");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("null base object")
	{
		Expression<ReflectedObject*> testExpression(&context, "nestedSelfObject.nestedSelfObject.getObject()");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("null base void")
	{
		Expression<void> testExpression(&context, "nestedSelfObject.nestedSelfObject.doSomething()");
		testExpression.getValue(&context);
		doCommonChecks(&testExpression, false, false, false, context);
	}
}


TEST_CASE("ExpressionAny", "[ExpressionAny]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	CatGenericType genericType = CatGenericType(objectTypeInfo);
	CatRuntimeContext context("builtinTests_Select", &errorManager);
	context.addScope(&reflectedObject, true);	

	SECTION("Literal Float")
	{
		ExpressionAny testExpression(&context, "1.1f");
		doChecks(1.1f, false, true, true, testExpression, context);
	}
	SECTION("Literal Int")
	{
		ExpressionAny testExpression(&context, "11");
		doChecks(11, false, true, true, testExpression, context);
	}
	SECTION("Literal Boolean true")
	{
		ExpressionAny testExpression(&context, "true");
		doChecks(true, false, true, true, testExpression, context);
	}
	SECTION("Literal Boolean false")
	{
		ExpressionAny testExpression(&context, "false");
		doChecks(false, false, true, true, testExpression, context);
	}
	SECTION("Literal String")
	{
		ExpressionAny testExpression(&context, "\"test\"");
		doChecks(std::string("test"), false, true, true, testExpression, context);
	}
	SECTION("Float Variable")
	{
		ExpressionAny testExpression(&context, "aFloat");
		doChecks(999.9f, false, false, false, testExpression, context);
	}
	SECTION("Int Variable")
	{
		ExpressionAny testExpression(&context, "theInt");
		doChecks(42, false, false, false, testExpression, context);
	}
	SECTION("Boolean Variable true")
	{
		ExpressionAny testExpression(&context, "aBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Boolean Variable false")
	{
		ExpressionAny testExpression(&context, "no");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("String Variable")
	{
		ExpressionAny testExpression(&context, "text");
		doChecks(std::string("Hello!"), false, false, false, testExpression, context);
	}
	SECTION("Object Variable")
	{
		ExpressionAny testExpression(&context, "nestedSelfObject");
		doChecks((Reflection::Reflectable*)reflectedObject.nestedSelfObject, false, false, false, testExpression, context);
	}
	SECTION("Vector Variable")
	{
		ExpressionAny testExpression(&context, "reflectableObjectsVector");
		doChecks(&reflectedObject.reflectableObjectsVector, false, false, false, testExpression, context);
	}
	SECTION("Map Variable")
	{
		ExpressionAny testExpression(&context, "reflectableObjectsMap");
		doChecks(&reflectedObject.reflectableObjectsMap, false, false, false, testExpression, context);
	}
	SECTION("Void")
	{
		ExpressionAny testExpression(&context, "doSomething()");
		doCommonChecks(&testExpression, false, false, false, context);
		REQUIRE(testExpression.getType().isVoidType());
		std::any value = testExpression.getValue(&context);
		CHECK_FALSE(value.has_value());
		std::any value2 = testExpression.getInterpretedValue(&context);
		CHECK_FALSE(value2.has_value());
	}
}

TEST_CASE("Custom Types", "[customtypes]")
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	std::unique_ptr<ReflectedObject> objectUniquePtr(new ReflectedObject());
	ExpressionErrorManager errorManager;
	TypeInfo* objectTypeInfo = TypeRegistry::get()->registerType<ReflectedObject>();
	CatGenericType genericType = CatGenericType(objectTypeInfo);
	const char* customTypeName2 = "MyType2";
	TypeRegistry::get()->removeType(customTypeName2);
	std::unique_ptr<CustomTypeInfo> customType(new CustomTypeInfo(customTypeName2));
	TypeRegistry::get()->registerType(customTypeName2, customType.get());
	customType->addFloatMember("myFloat", 0.001f);
	customType->addIntMember("myInt", 54321);
	customType->addStringMember("myString", "foo");
	customType->addBoolMember("myBoolean", true);
	customType->addObjectMember("myObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject", &reflectedObject, objectTypeInfo);
	customType->addObjectMember("myNullObject2", objectUniquePtr.get(), objectTypeInfo);
	std::unique_ptr<CustomTypeInstance> typeInstance(customType->createInstance());

	const char* customTypeName3 = "MyType3";
	TypeRegistry::get()->removeType(customTypeName3);
	std::unique_ptr<CustomTypeInfo> customType2(new CustomTypeInfo(customTypeName3));
	TypeRegistry::get()->registerType(customTypeName3, customType2.get());
	customType2->addFloatMember("myNullFloat", 0.001f);
	customType2->addIntMember("myNullInt", 54321);
	customType2->addStringMember("myNullString", "foo");
	customType2->addBoolMember("myNullBoolean", true);
	customType2->addObjectMember("myNullObject3", &reflectedObject, objectTypeInfo);

	//The case where the pointer is set to null manually
	std::any instanceAny((Reflectable*)typeInstance.get());
	std::any nullAny((Reflectable*)nullptr);
	static_cast<CustomTypeObjectMemberInfo*>(typeInstance->typeInfo->getMemberInfo("myNullObject"))->assign(instanceAny, nullAny);
	//The case where the reflectable handle is set to null through deletion of the observed object.
	objectUniquePtr.reset(nullptr);

	CatRuntimeContext context("builtinTests_Select", &errorManager);
	context.addScope(&reflectedObject, true);
	context.addCustomTypeScope(customType2.get());
	context.addCustomTypeScope(customType.get(), typeInstance.get());

	SECTION("Float Variable")
	{
		Expression<float> testExpression(&context, "myFloat");
		doChecks(0.001f, false, false, false, testExpression, context);
	}
	SECTION("Int Variable")
	{
		Expression<int> testExpression(&context, "myInt");
		doChecks(54321, false, false, false, testExpression, context);
	}
	SECTION("String Variable")
	{
		Expression<std::string> testExpression(&context, "myString");
		doChecks(std::string("foo"), false, false, false, testExpression, context);
	}
	SECTION("Boolean Variable")
	{
		Expression<bool> testExpression(&context, "myBoolean");
		doChecks(true, false, false, false, testExpression, context);
	}
	SECTION("Object Variable")
	{
		Expression<ReflectedObject*> testExpression(&context, "myObject");
		doChecks(&reflectedObject, false, false, false, testExpression, context);
	}
	SECTION("Null object Variable")
	{
		Expression<ReflectedObject*> testExpression(&context, "myNullObject.nullObject");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("Null object Variable")
	{
		Expression<ReflectedObject*> testExpression(&context, "myNullObject2.nullObject");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}

	SECTION("Null base float Variable")
	{
		Expression<float> testExpression(&context, "myNullFloat");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Null base int Variable")
	{
		Expression<int> testExpression(&context, "myNullInt");
		doChecks(0, false, false, false, testExpression, context);
	}
	SECTION("Null base string Variable")
	{
		Expression<std::string> testExpression(&context, "myNullString");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("Null base boolean Variable")
	{
		Expression<bool> testExpression(&context, "myNullBoolean");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Null base object Variable")
	{
		Expression<ReflectedObject*> testExpression(&context, "myNullObject3");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}

	SECTION("Added Float Variable")
	{
		customType->addFloatMember("anotherFloat", 1.234f);
		Expression<float> testExpression(&context, "anotherFloat");
		doChecks(1.234f, false, false, false, testExpression, context);
	}
	SECTION("Added Int Variable")
	{
		customType->addIntMember("anotherInt", 12345);
		Expression<int> testExpression(&context, "anotherInt");
		doChecks(12345, false, false, false, testExpression, context);
	}
	SECTION("Added String Variable")
	{
		customType->addStringMember("anotherString", "bar");
		Expression<std::string> testExpression(&context, "anotherString");
		doChecks(std::string("bar"), false, false, false, testExpression, context);
	}
	SECTION("Added Boolean Variable")
	{
		customType->addBoolMember("anotherBoolean", false);
		Expression<bool> testExpression(&context, "anotherBoolean");
		doChecks(false, false, false, false, testExpression, context);
	}
	SECTION("Added Object Variable")
	{
		customType->addObjectMember("anotherObject", reflectedObject.nestedSelfObject, objectTypeInfo);
		Expression<ReflectedObject*> testExpression(&context, "anotherObject");
		doChecks(reflectedObject.nestedSelfObject, false, false, false, testExpression, context);
	}
}
