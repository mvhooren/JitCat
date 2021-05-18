/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ExpressionAny.h"
#include "jitcat/CatASTNodes.h"
#include "jitcat/Configuration.h"
#include "jitcat/Document.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/JitCat.h"
#include "jitcat/SLRParseResult.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::AST;


ExpressionAny::ExpressionAny():
	getValuePtr(&ExpressionAny::getDefaultValue),
	nativeFunctionAddress(0)
{
}


ExpressionAny::ExpressionAny(const char* expression):
	ExpressionBase(expression),
	getValuePtr(&ExpressionAny::getDefaultValue),
	nativeFunctionAddress(0)
{
}


ExpressionAny::ExpressionAny(const std::string& expression):
	ExpressionBase(expression),
	getValuePtr(&ExpressionAny::getDefaultValue),
	nativeFunctionAddress(0)
{
}


ExpressionAny::ExpressionAny(CatRuntimeContext* compileContext, const std::string& expression):
	ExpressionBase(compileContext, expression),
	getValuePtr(&ExpressionAny::getDefaultValue),
	nativeFunctionAddress(0)
{
	compile(compileContext);
}


const std::any ExpressionAny::getValue(CatRuntimeContext* runtimeContext)
{
	return (this->*getValuePtr)(runtimeContext);
}


const std::any jitcat::ExpressionAny::getInterpretedValue(CatRuntimeContext* runtimeContext)
{
	if (isConstant)
	{
		return cachedValue;
	}
	else if (parseResult.astRootNode != nullptr)
	{
		std::any result = parseResult.getNode<CatTypedExpression>()->execute(runtimeContext);
		runtimeContext->clearTemporaries();
		return result;
	}
	else
	{
		return std::any();
	}
}


void ExpressionAny::compile(CatRuntimeContext* context)
{
	if (context == nullptr)
	{
		context = &CatRuntimeContext::getDefaultContext();
		context->getErrorManager()->clear();
	}
	if (parse(context, context->getErrorManager(), this, CatGenericType()))
	{
		if (isConstant)
		{
			cachedValue = parseResult.getNode<CatTypedExpression>()->execute(context);
			getValuePtr = &ExpressionAny::getCachedValue;
		}
		else if (!Configuration::enableLLVM && !Configuration::usePreCompiledExpressions)
		{
			getValuePtr = &ExpressionAny::getExecuteInterpretedValue;
		}
	}
	else
	{
		resetCompiledFunctionToDefault();
	}
}


void ExpressionAny::handleCompiledFunction(uintptr_t functionAddress)
{
	nativeFunctionAddress = functionAddress;
	if (nativeFunctionAddress == 0)
	{
		getValuePtr = &ExpressionAny::getExecuteInterpretedValue;
	}
	else
	{
		if		(valueType.isIntType())						getValuePtr = &ExpressionAny::getExecuteIntValue;
		else if (valueType.isVoidType())					getValuePtr = &ExpressionAny::getExecuteVoidValue;
		else if (valueType.isFloatType())					getValuePtr = &ExpressionAny::getExecuteFloatValue;
		else if (valueType.isDoubleType())					getValuePtr = &ExpressionAny::getExecuteDoubleValue;
		else if (valueType.isBoolType())					getValuePtr = &ExpressionAny::getExecuteBoolValue;
		else if (valueType.isReflectablePointerOrHandle())	getValuePtr = &ExpressionAny::getExecuteReflectablePtrValue;
		else if (valueType.isPointerToPointerType() && valueType.getPointeeType()->isPointerToReflectableObjectType())
		{
			getValuePtr = &ExpressionAny::getExecutePtrPtrValue;
		}
		else 
		{
			assert(false);
			getValuePtr = &ExpressionAny::getDefaultValue;
		}
	}
}


void ExpressionAny::resetCompiledFunctionToDefault()
{
	nativeFunctionAddress = 0;
	getValuePtr = &ExpressionAny::getDefaultValue;
}


const std::any jitcat::ExpressionAny::getExecuteVoidValue(CatRuntimeContext* runtimeContext)
{
	if (runtimeContext == nullptr)	runtimeContext = &CatRuntimeContext::getDefaultContext();
	reinterpret_cast<void(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext); 
	return std::any();
}


const std::any jitcat::ExpressionAny::getExecuteBoolValue(CatRuntimeContext* runtimeContext)
{
	if (runtimeContext == nullptr)	runtimeContext = &CatRuntimeContext::getDefaultContext();
	return std::any(reinterpret_cast<bool(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
}


const std::any jitcat::ExpressionAny::getExecuteIntValue(CatRuntimeContext* runtimeContext)
{
	if (runtimeContext == nullptr)	runtimeContext = &CatRuntimeContext::getDefaultContext();
	return std::any(reinterpret_cast<int(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
}


const std::any jitcat::ExpressionAny::getExecuteFloatValue(CatRuntimeContext* runtimeContext)
{
	if (runtimeContext == nullptr)	runtimeContext = &CatRuntimeContext::getDefaultContext();
	return std::any(reinterpret_cast<float(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
}


const std::any jitcat::ExpressionAny::getExecuteDoubleValue(CatRuntimeContext* runtimeContext)
{
	if (runtimeContext == nullptr)	runtimeContext = &CatRuntimeContext::getDefaultContext();
	return std::any(reinterpret_cast<double(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
}


const std::any jitcat::ExpressionAny::getExecuteReflectablePtrValue(CatRuntimeContext* runtimeContext)
{
	if (runtimeContext == nullptr)	runtimeContext = &CatRuntimeContext::getDefaultContext();
	return valueType.getPointeeType()->getObjectType()->getTypeCaster()->castFromRawPointer(reinterpret_cast<uintptr_t(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
}


const std::any jitcat::ExpressionAny::getExecutePtrPtrValue(CatRuntimeContext* runtimeContext)
{
	if (runtimeContext == nullptr)	runtimeContext = &CatRuntimeContext::getDefaultContext();
	return valueType.getPointeeType()->getPointeeType()->getObjectType()->getTypeCaster()->castFromRawPointerPointer(reinterpret_cast<uintptr_t(*)(CatRuntimeContext*)>(nativeFunctionAddress)(runtimeContext));
}


const std::any ExpressionAny::getExecuteInterpretedValue(CatRuntimeContext* runtimeContext)
{
	if (runtimeContext == nullptr)	runtimeContext = &CatRuntimeContext::getDefaultContext();
	std::any result = parseResult.getNode<CatTypedExpression>()->execute(runtimeContext);
	runtimeContext->clearTemporaries();
	return result;
}


const std::any ExpressionAny::getCachedValue(CatRuntimeContext* runtimeContext)
{
	return cachedValue;
}


const std::any ExpressionAny::getDefaultValue(CatRuntimeContext* runtimeContext)
{
	return std::any();
}
