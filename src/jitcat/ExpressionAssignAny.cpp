#include "jitcat/ExpressionAssignAny.h"
#include "jitcat/AssignableType.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/Configuration.h"
#include "jitcat/SLRParseResult.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/TypeTraits.h"


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Parser;
using namespace jitcat::Reflection;


ExpressionAssignAny::ExpressionAssignAny():
	ExpressionBase(true),
	nativeFunctionAddress(0)
{
}


ExpressionAssignAny::ExpressionAssignAny(const char* expression):
	ExpressionBase(expression, true),
	nativeFunctionAddress(0)
{
}


ExpressionAssignAny::ExpressionAssignAny(const std::string& expression):
	ExpressionBase(expression, true),
	nativeFunctionAddress(0)
{
}


ExpressionAssignAny::ExpressionAssignAny(CatRuntimeContext* compileContext, const std::string& expression):
	ExpressionBase(compileContext, expression, true),
	nativeFunctionAddress(0)
{
	compile(compileContext);
}


bool ExpressionAssignAny::assignValue(CatRuntimeContext* runtimeContext, std::any value, const CatGenericType& valueType)
{
	if constexpr (Configuration::enableLLVM)
	{
		if (parseResult->astRootNode != nullptr)
		{
			const CatGenericType myType = getType();
			std::any convertedValue = myType.convertToType(value, valueType);
			if (myType.isIntType())	reinterpret_cast<void(*)(CatRuntimeContext*, int)>(nativeFunctionAddress)(runtimeContext, std::any_cast<int>(convertedValue));
			else if (myType.isFloatType())	reinterpret_cast<void(*)(CatRuntimeContext*, float)>(nativeFunctionAddress)(runtimeContext, std::any_cast<float>(convertedValue));
			else if (myType.isBoolType())	reinterpret_cast<void(*)(CatRuntimeContext*, bool)>(nativeFunctionAddress)(runtimeContext, std::any_cast<bool>(convertedValue));
			else if (myType.isStringType())	reinterpret_cast<void(*)(CatRuntimeContext*, const std::string&)>(nativeFunctionAddress)(runtimeContext, std::any_cast<std::string>(convertedValue));
			else if (myType.isObjectType())
			{
				//Use the type caster to cast the object contained in value to a std::any containing a Reflectable*;
				//std::any reflectableAny = valueType.getObjectType()->getTypeCaster()->cast(value);
				reinterpret_cast<void(*)(CatRuntimeContext*, Reflectable*)>(nativeFunctionAddress)(runtimeContext, std::any_cast<Reflectable*>(value));
			}
			else
			{
				return false;
			}
		}
		return !hasError();
	}
	else
	{
		return assignInterpretedValue(runtimeContext, value, valueType);
	}
	return false;
}


bool ExpressionAssignAny::assignInterpretedValue(CatRuntimeContext* runtimeContext, std::any value, const CatGenericType& valueType)
{
	if (parseResult->astRootNode != nullptr && parseResult->getNode<AST::CatTypedExpression>()->isAssignable())
	{
		jitcat::AST::CatAssignableExpression* assignable = parseResult->getNode<AST::CatAssignableExpression>();
		Reflection::AssignableType assignableType = Reflection::AssignableType::None;
		std::any target = assignable->executeAssignable(runtimeContext, assignableType);
		value = getType().convertToType(value, valueType);
		jitcat::AST::ASTHelper::doAssignment(target, value, getType().toWritable(), assignableType);
		return true;
	}
	return false;
}


void ExpressionAssignAny::compile(CatRuntimeContext* context)
{
	parse(context, context->getErrorManager(), this, CatGenericType());
}


void ExpressionAssignAny::handleCompiledFunction(uintptr_t functionAddress)
{
	nativeFunctionAddress = functionAddress;
}


bool jitcat::ExpressionAssignAny::assignUncastedPointer(CatRuntimeContext* runtimeContext, std::any pointerValue, const CatGenericType& valueType)
{
	std::any reflectableAny = valueType.getObjectType()->getTypeCaster()->cast(pointerValue);
	return assignValue(runtimeContext, reflectableAny, valueType);
}


bool jitcat::ExpressionAssignAny::assignInterpretedUncastedPointer(CatRuntimeContext* runtimeContext, std::any pointerValue, const CatGenericType& valueType)
{
	std::any reflectableValue = valueType.getObjectType()->getTypeCaster()->cast(pointerValue);
	return assignInterpretedValue(runtimeContext, reflectableValue, valueType);
}
