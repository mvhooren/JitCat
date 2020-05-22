/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ExpressionAssignAny.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/Configuration.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/SLRParseResult.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/TypeTraits.h"


using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Parser;
using namespace jitcat::Reflection;


ExpressionAssignAny::ExpressionAssignAny():
	ExpressionBase(true),
	nativeFunctionAddress(0),
	assignmentOperatorFunction(nullptr)
{
}


ExpressionAssignAny::ExpressionAssignAny(const char* expression):
	ExpressionBase(expression, true),
	nativeFunctionAddress(0),
	assignmentOperatorFunction(nullptr)
{
}


ExpressionAssignAny::ExpressionAssignAny(const std::string& expression):
	ExpressionBase(expression, true),
	nativeFunctionAddress(0),
	assignmentOperatorFunction(nullptr)
{
}


ExpressionAssignAny::ExpressionAssignAny(CatRuntimeContext* compileContext, const std::string& expression):
	ExpressionBase(compileContext, expression, true),
	nativeFunctionAddress(0),
	assignmentOperatorFunction(nullptr)
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
			else if (myType.isDoubleType())	reinterpret_cast<void(*)(CatRuntimeContext*, double)>(nativeFunctionAddress)(runtimeContext, std::any_cast<double>(convertedValue));
			else if (myType.isBoolType())	reinterpret_cast<void(*)(CatRuntimeContext*, bool)>(nativeFunctionAddress)(runtimeContext, std::any_cast<bool>(convertedValue));
			else if (myType.isStringType())	reinterpret_cast<void(*)(CatRuntimeContext*, const Configuration::CatString&)>(nativeFunctionAddress)(runtimeContext, std::any_cast<Configuration::CatString>(convertedValue));
			else if (myType.isReflectableHandleType() || myType.isPointerToReflectableObjectType())
			{
				reinterpret_cast<void(*)(CatRuntimeContext*, uintptr_t)>(nativeFunctionAddress)(runtimeContext, myType.getRawPointer(value));
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


bool ExpressionAssignAny::assignInterpretedValue(CatRuntimeContext* runtimeContext, std::any value, const CatGenericType& rValueType)
{
	if (parseResult->astRootNode != nullptr && parseResult->getNode<AST::CatTypedExpression>()->isAssignable())
	{
		jitcat::AST::CatAssignableExpression* assignable = parseResult->getNode<AST::CatAssignableExpression>();
		if (assignmentOperatorFunction != nullptr)
		{
			if (valueType.compare(rValueType, false, true))
			{
				std::any assignableValue = assignable->execute(runtimeContext);
				assignmentOperatorFunction->call(runtimeContext, assignableValue, {value});
				runtimeContext->clearTemporaries();
				return true;
			}
			else if (valueType.compare(rValueType, false, false))
			{
				IndirectionConversionMode conversionMode = valueType.getIndirectionConversion(rValueType);
				if (isValidConversionMode(conversionMode))
				{
					std::any convertedValue = valueType.doIndirectionConversion(value, conversionMode);
					std::any assignableValue = assignable->execute(runtimeContext);
					assignmentOperatorFunction->call(runtimeContext, assignableValue, {convertedValue});
					runtimeContext->clearTemporaries();
					return true;
				}
			}
		}
		else
		{
			std::any target = assignable->executeAssignable(runtimeContext);
			value = getType().convertToType(value, rValueType);
			jitcat::AST::ASTHelper::doAssignment(target, value, getType().toWritable().toPointer(), rValueType);
			runtimeContext->clearTemporaries();
			return true;
		}
	}
	return false;
}


void ExpressionAssignAny::compile(CatRuntimeContext* context)
{
	parse(context, context->getErrorManager(), this, CatGenericType());
	if (parseResult->astRootNode != nullptr && parseResult->getNode<AST::CatTypedExpression>()->isAssignable())
	{
		if (getType().isPointerToReflectableObjectType() && getType().getOwnershipSemantics() == TypeOwnershipSemantics::Value)
		{
			SearchFunctionSignature signature("=", {getType().removeIndirection().toPointer()});
			assignmentOperatorFunction = getType().getPointeeType()->getObjectType()->getMemberFunctionInfo(signature);
			if (assignmentOperatorFunction == nullptr)
			{
				parseResult->astRootNode.reset(nullptr);
				parseResult->success = false;
			}
		}
	}
}


void ExpressionAssignAny::handleCompiledFunction(uintptr_t functionAddress)
{
	nativeFunctionAddress = functionAddress;
}


bool jitcat::ExpressionAssignAny::assignUncastedPointer(CatRuntimeContext* runtimeContext, uintptr_t pointerValue, const CatGenericType& valueType)
{
	std::any reflectableAny = valueType.getPointeeType()->getObjectType()->getTypeCaster()->castFromRawPointer(pointerValue);
	return assignValue(runtimeContext, reflectableAny, valueType);
}


bool jitcat::ExpressionAssignAny::assignInterpretedUncastedPointer(CatRuntimeContext* runtimeContext, uintptr_t pointerValue, const CatGenericType& valueType)
{
	std::any reflectableValue = valueType.getPointeeType()->getObjectType()->getTypeCaster()->castFromRawPointer(pointerValue);
	return assignInterpretedValue(runtimeContext, reflectableValue, valueType);
}
