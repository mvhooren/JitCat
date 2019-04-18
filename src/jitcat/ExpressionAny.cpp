/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ExpressionAny.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/CatASTNodes.h"
#include "jitcat/Configuration.h"
#include "jitcat/Document.h"
#include "jitcat/JitCat.h"
#include "jitcat/SLRParseResult.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::AST;


ExpressionAny::ExpressionAny():
	nativeFunctionAddress(0)
{
}


ExpressionAny::ExpressionAny(const char* expression):
	ExpressionBase(expression),
	nativeFunctionAddress(0)
{
}


ExpressionAny::ExpressionAny(const std::string& expression):
	ExpressionBase(expression),
	nativeFunctionAddress(0)
{
}


ExpressionAny::ExpressionAny(CatRuntimeContext* compileContext, const std::string& expression):
	ExpressionBase(compileContext, expression),
	nativeFunctionAddress(0)
{
	compile(compileContext);
}


const std::any ExpressionAny::getValue(CatRuntimeContext* runtimeContext)
{
	if (isConstant)
	{
		return cachedValue;
	}
	else if (parseResult->astRootNode != nullptr)
	{
		if constexpr (Configuration::enableLLVM)
		{
			if		(valueType.isIntType())		return std::any(reinterpret_cast<int(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isFloatType())	return std::any(reinterpret_cast<float(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isBoolType())	return std::any(reinterpret_cast<bool(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isStringType())	return std::any(reinterpret_cast<std::string(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isObjectType())	return valueType.getObjectType()->getTypeCaster()->cast(reinterpret_cast<uintptr_t(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isVectorType())	return valueType.getContainerManipulator()->createAnyPointer(reinterpret_cast<uintptr_t(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isMapType())		return valueType.getContainerManipulator()->createAnyPointer(reinterpret_cast<uintptr_t(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else 
			{
				return std::any();
			}
		}
		else
		{
			return parseResult->getNode<CatTypedExpression>()->execute(runtimeContext);
		}
	}
	else
	{
		return std::any();
	}
}


const std::any jitcat::ExpressionAny::getInterpretedValue(CatRuntimeContext* runtimeContext)
{
	if (isConstant)
	{
		return cachedValue;
	}
	else if (parseResult->astRootNode != nullptr)
	{
		return parseResult->getNode<CatTypedExpression>()->execute(runtimeContext);
	}
	else
	{
		return std::any();
	}
}


void ExpressionAny::compile(CatRuntimeContext* context)
{
	if (parse(context, CatGenericType()) && isConstant)
	{
		cachedValue = parseResult->getNode<CatTypedExpression>()->execute(context);
	}
}


void ExpressionAny::handleCompiledFunction(uintptr_t functionAddress)
{
	nativeFunctionAddress = functionAddress;
}
