/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ExpressionAny.h"
#include "ExpressionErrorManager.h"
#include "CatASTNodes.h"
#include "Configuration.h"
#include "Document.h"
#include "JitCat.h"
#include "SLRParseResult.h"
#include "Tools.h"
#include "TypeInfo.h"


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
	else if (expressionAST != nullptr)
	{
		if constexpr (Configuration::enableLLVM)
		{
			if		(valueType.isIntType())		return std::any(reinterpret_cast<int(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isFloatType())	return std::any(reinterpret_cast<float(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isBoolType())	return std::any(reinterpret_cast<bool(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isStringType())	return std::any(reinterpret_cast<std::string(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isObjectType())	return valueType.getObjectType()->getTypeCaster()->cast(reinterpret_cast<uintptr_t(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isVectorType())	return valueType.getObjectType()->getTypeCaster()->castToVectorOf(reinterpret_cast<uintptr_t(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else if (valueType.isMapType())		return valueType.getObjectType()->getTypeCaster()->castToStringIndexedMapOf(reinterpret_cast<uintptr_t(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
			else 
			{
				return std::any();
			}
		}
		else
		{
			return expressionAST->execute(runtimeContext);
		}
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
		cachedValue = expressionAST->execute(context);
	}
}


void ExpressionAny::handleCompiledFunction(uintptr_t functionAddress)
{
	nativeFunctionAddress = functionAddress;
}
