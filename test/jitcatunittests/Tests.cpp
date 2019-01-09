#include <catch2\catch.hpp>
#include "CatRuntimeContext.h"
#include "Expression.h"
#include "LLVMCatIntrinsics.h"
#include "TestObjects.h"
#include "Tools.h"

#include <functional>
#include <cmath>

using namespace TestObjects;


template<typename T>
bool doCommonChecks(Expression<T>& expression, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, CatRuntimeContext& context)
{
	if (!shouldHaveError)
	{
		REQUIRE_FALSE(expression.hasError());
	}
	else
	{
		REQUIRE(expression.hasError());
		if constexpr (!std::is_same<T, void>::value)
		{
			//When te expression has an error, the getValue functions should return the default value
			CHECK(expression.getValue(&context) == T());
			CHECK(expression.getInterpretedValue(&context) == T());
		}
		return false;
	}
	if (shouldBeConst)
	{
		CHECK(expression.isConst());
	}
	else
	{
		CHECK_FALSE(expression.isConst());
	}
	if (shouldBeLiteral)
	{
		CHECK(expression.isLiteral());
	}
	else
	{
		CHECK_FALSE(expression.isLiteral());
	}
	return true;
}


template <typename T>
void doChecksFn(std::function<bool(const T&)> valueCheck, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, Expression<T>& expression, CatRuntimeContext& context)
{
	if (doCommonChecks(expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context))
	{
		CHECK(valueCheck(expression.getValue(&context)));
		CHECK(valueCheck(expression.getInterpretedValue(&context)));
	}
}


template <typename T>
void doChecks(const T& expectedValue, bool shouldHaveError, bool shouldBeConst, bool shouldBeLiteral, Expression<T>& expression, CatRuntimeContext& context)
{
	if (doCommonChecks(expression, shouldHaveError, shouldBeConst, shouldBeLiteral, context))
	{
		if constexpr (std::is_same<T, float>::value)
		{
			CHECK(expression.getValue(&context) == Approx(expectedValue));
			CHECK(expression.getInterpretedValue(&context) == Approx(expectedValue));
		}
		else
		{
			CHECK(expression.getValue(&context) == expectedValue);
			CHECK(expression.getInterpretedValue(&context) == expectedValue);
		}
	}
}



TEST_CASE("Floating Point Tests", "[float][operators]" ) 
{
	ReflectedObject reflectedObject;
	ExpressionErrorManager errorManager;
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "floatTests", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "intTests", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "intTests", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

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


TEST_CASE("Operator precedence", "[containers][map]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Select", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_ToInt", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_ToFloat", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_ToString", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_ToPrettyString", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_ToFixedLengthString", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_ToBool", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Sin", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

	SECTION("Sin_Constant")
	{
		Expression<float> testExpression(&context, "sin(42.0f)");
		doChecks(sin(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sin_Negative Constant")
	{
		Expression<float> testExpression(&context, "sin(-42.0f)");
		doChecks(sin(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sin_IntConstant")
	{
		Expression<float> testExpression(&context, "sin(3)");
		doChecks(sin((float)3), false, true, false, testExpression, context);
	}
	SECTION("Sin_Variable")
	{
		Expression<float> testExpression(&context, "sin(aFloat)");
		doChecks(sin(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sin_Negative Variable")
	{
		Expression<float> testExpression(&context, "sin(-aFloat)");
		doChecks(sin(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sin_IntVariable")
	{
		Expression<float> testExpression(&context, "sin(theInt)");
		doChecks(sin((float)reflectedObject.theInt), false, false, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Cos", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

	SECTION("Cos_Constant")
	{
		Expression<float> testExpression(&context, "cos(42.0f)");
		doChecks(cos(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Cos_Negative Constant")
	{
		Expression<float> testExpression(&context, "cos(-42.0f)");
		doChecks(cos(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Cos_IntConstant")
	{
		Expression<float> testExpression(&context, "cos(3)");
		doChecks(cos((float)3), false, true, false, testExpression, context);
	}
	SECTION("Cos_Variable")
	{
		Expression<float> testExpression(&context, "cos(aFloat)");
		doChecks(cos(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Cos_Negative Variable")
	{
		Expression<float> testExpression(&context, "cos(-aFloat)");
		doChecks(cos(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Cos_IntVariable")
	{
		Expression<float> testExpression(&context, "cos(theInt)");
		doChecks(cos((float)reflectedObject.theInt), false, false, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Tan", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

	SECTION("Tan_Constant")
	{
		Expression<float> testExpression(&context, "tan(42.0f)");
		doChecks(tan(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Tan_Negative Constant")
	{
		Expression<float> testExpression(&context, "tan(-42.0f)");
		doChecks(tan(-42.0f), false, true, false, testExpression, context);
	}
	SECTION("Tan_IntConstant")
	{
		Expression<float> testExpression(&context, "tan(3)");
		doChecks(tan((float)3), false, true, false, testExpression, context);
	}
	SECTION("Tan_Variable")
	{
		Expression<float> testExpression(&context, "tan(aFloat)");
		doChecks(tan(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Tan_Negative Variable")
	{
		Expression<float> testExpression(&context, "tan(-aFloat)");
		doChecks(tan(-reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Tan_IntVariable")
	{
		Expression<float> testExpression(&context, "tan(theInt)");
		doChecks(tan((float)reflectedObject.theInt), false, false, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Abs", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

	SECTION("Abs_Constant")
	{
		Expression<float> testExpression(&context, "abs(42.0f)");
		doChecks(abs(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Abs_Negative Constant")
	{
		Expression<float> testExpression(&context, "abs(-42.0f)");
		doChecks(abs(-42.0f), false, true, false, testExpression, context);
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
		doChecks(abs(-reflectedObject.aFloat), false, false, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Sqrt", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

	SECTION("Sqrt_Constant")
	{
		Expression<float> testExpression(&context, "sqrt(42.0f)");
		doChecks(sqrt(42.0f), false, true, false, testExpression, context);
	}
	SECTION("Sqrt_Negative Constant")
	{
		Expression<float> testExpression(&context, "sqrt(-42.0f)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, true, false, testExpression, context);
	}
	SECTION("Sqrt_IntConstant")
	{
		Expression<float> testExpression(&context, "sqrt(3)");
		doChecks(sqrt(3.0f), false, true, false, testExpression, context);
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
		doChecks(sqrt(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Sqrt_Negative Variable")
	{
		Expression<float> testExpression(&context, "sqrt(-aFloat)");
		doChecksFn<float>([](float value){return std::isnan(value);}, false, false, false, testExpression, context);
	}
	SECTION("Sqrt_IntVariable")
	{
		Expression<float> testExpression(&context, "sqrt(theInt)");
		doChecks(sqrt((float)reflectedObject.theInt), false, false, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Random", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_RandomRange", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Round", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_StringRound", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Cap", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Min", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Max", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Log", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Pow", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

	SECTION("Pow_cc1")
	{
		Expression<float> testExpression(&context, "pow(11.1f, 1)");
		doChecks(pow(11.1f, 1), false, true, false, testExpression, context);
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
		doChecks(pow(-11.1f, 999.0f), false, true, false, testExpression, context);
	}
	SECTION("Pow_float1")
	{
		Expression<float> testExpression(&context, "pow(aFloat, 0)");
		doChecks(1.0f, false, false, false, testExpression, context);
	}
	SECTION("Pow_float2")
	{
		Expression<float> testExpression(&context, "pow(aFloat, 1000.0f)");
		doChecks(pow(reflectedObject.aFloat, 1000.0f), false, false, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Ceil", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

	SECTION("Ceil_Constant")
	{
		Expression<float> testExpression(&context, "ceil(42.1f)");
		doChecks(ceil(42.1f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_Negative Constant")
	{
		Expression<float> testExpression(&context, "ceil(-42.1f)");
		doChecks(ceil(-42.1f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_IntConstant")
	{
		Expression<float> testExpression(&context, "ceil(3)");
		doChecks(ceil(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_Negative IntConstant")
	{
		Expression<float> testExpression(&context, "ceil(-3)");
		doChecks(ceil(-3.0f), false, true, false, testExpression, context);
	}
	SECTION("Ceil_Zero Variable")
	{
		Expression<float> testExpression(&context, "ceil(zeroFloat)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Ceil_Variable")
	{
		Expression<float> testExpression(&context, "ceil(aFloat)");
		doChecks(ceil(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Ceil_Negative Variable")
	{
		Expression<float> testExpression(&context, "ceil(-aFloat)");
		doChecks(ceil(-reflectedObject.aFloat), false, false, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Floor", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);	

	SECTION("Floor_Constant")
	{
		Expression<float> testExpression(&context, "floor(42.1f)");
		doChecks(floor(42.1f), false, true, false, testExpression, context);
	}
	SECTION("Floor_Negative Constant")
	{
		Expression<float> testExpression(&context, "floor(-42.1f)");
		doChecks(floor(-42.1f), false, true, false, testExpression, context);
	}
	SECTION("Floor_IntConstant")
	{
		Expression<float> testExpression(&context, "floor(3)");
		doChecks(floor(3.0f), false, true, false, testExpression, context);
	}
	SECTION("Floor_Negative IntConstant")
	{
		Expression<float> testExpression(&context, "floor(-3)");
		doChecks(floor(-3.0f), false, true, false, testExpression, context);
	}
	SECTION("Floor_Zero Variable")
	{
		Expression<float> testExpression(&context, "floor(zeroFloat)");
		doChecks(0.0f, false, false, false, testExpression, context);
	}
	SECTION("Floor_Variable")
	{
		Expression<float> testExpression(&context, "floor(aFloat)");
		doChecks(floor(reflectedObject.aFloat), false, false, false, testExpression, context);
	}
	SECTION("Floor_Negative Variable")
	{
		Expression<float> testExpression(&context, "floor(-aFloat)");
		doChecks(floor(-reflectedObject.aFloat), false, false, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_FindInString", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_ReplaceInString", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_StringLength", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_SubString", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

	SECTION("SubString_cc1")
	{
		Expression<std::string> testExpression(&context, "subString(11.1f, 1, 3)");
		doChecks(std::string("1.1"), false, true, false, testExpression, context);
	}
	SECTION("SubString_cc2")
	{
		Expression<std::string> testExpression(&context, "subString(0, 11.0f, 12)");
		doChecks(std::string(""), true, true, false, testExpression, context);
	}
	SECTION("SubString_cc3")
	{
		Expression<std::string> testExpression(&context, "subString(\"9\", 9, 0)");
		doChecks(std::string(""), true, true, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Select", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Select", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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


TEST_CASE("Containers tests: Vector", "[containers][vector]" ) 
{
	ReflectedObject reflectedObject;
	reflectedObject.createNestedObjects();
	ExpressionErrorManager errorManager;
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Select", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

	SECTION("Vector get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsVector[0]");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Select", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

	SECTION("Map get object")
	{
		Expression<NestedReflectedObject*> testExpression(&context, "reflectableObjectsMap[0]");
		doChecks(reflectedObject.nestedObjectPointer, false, false, false, testExpression, context);
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
	CatRuntimeContext context(TypeRegistry::get()->registerType<ReflectedObject>(), nullptr, nullptr, nullptr, "builtinTests_Select", true, &errorManager);
	ObjectMemberReference<ReflectedObject>* globalsReference = new ObjectMemberReference<ReflectedObject>(&reflectedObject, nullptr, TypeRegistry::get()->registerType<ReflectedObject>());
	context.setGlobalReference(globalsReference);

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
		doCommonChecks(testExpression, false, false, false, context);
	}
	SECTION("Const void function")
	{
		Expression<void> testExpression(&context, "doSomethingConst()");
		testExpression.getValue(&context);
		doCommonChecks(testExpression, false, false, false, context);
	}
	SECTION("Void function 2")
	{
		Expression<void> testExpression(&context, "checkTheseValues(aBoolean, theInt, text, nestedSelfObject)");
		testExpression.getValue(&context);
		doCommonChecks(testExpression, false, false, false, context);
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
		doCommonChecks(testExpression, false, false, false, context);
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
	SECTION("null base boolean")
	{
		Expression<std::string> testExpression(&context, "nestedSelfObject.nestedSelfObject.getString()");
		doChecks(std::string(""), false, false, false, testExpression, context);
	}
	SECTION("null base boolean")
	{
		Expression<ReflectedObject*> testExpression(&context, "nestedSelfObject.nestedSelfObject.getObject()");
		doChecks<ReflectedObject*>(nullptr, false, false, false, testExpression, context);
	}
	SECTION("null base void")
	{
		Expression<void> testExpression(&context, "nestedSelfObject.nestedSelfObject.doSomething()");
		testExpression.getValue(&context);
		doCommonChecks(testExpression, false, false, false, context);
	}
}