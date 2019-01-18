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
			switch(valueType.getCatType())
			{
				case CatType::Int:		return std::any(reinterpret_cast<int(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
				case CatType::Float:	return std::any(reinterpret_cast<float(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
				case CatType::Bool:		return std::any(reinterpret_cast<bool(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
				case CatType::String:	return std::any(reinterpret_cast<std::string(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
				case CatType::Object:
				{
					uintptr_t objectPointer = reinterpret_cast<uintptr_t(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext);

					if (valueType.isObjectType())		return valueType.getObjectType()->getTypeCaster()->cast(objectPointer);
					else if (valueType.isVectorType())	return valueType.getObjectType()->getTypeCaster()->castToVectorOf(objectPointer);
					else if (valueType.isMapType())		return valueType.getObjectType()->getTypeCaster()->castToStringIndexedMapOf(objectPointer);
					else								return  std::any();
				}
				default:
				case CatType::Unknown:
				case CatType::Void:		return std::any();
			}
		}
		else
		{
			return expressionAST->execute(runtimeContext).toAny();
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
		cachedValue = expressionAST->execute(context).toAny();
	}
}


void ExpressionAny::handleCompiledFunction(uintptr_t functionAddress)
{
	nativeFunctionAddress = functionAddress;
}
