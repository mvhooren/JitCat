/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ExpressionAny.h"
#include "ExpressionErrorManager.h"
#include "CatASTNodes.h"
#include "Document.h"
#include "JitCat.h"
#include "SLRParseResult.h"
#include "Tools.h"


ExpressionAny::ExpressionAny()
{
}


ExpressionAny::ExpressionAny(const char* expression):
	ExpressionBase(expression)
{
}


ExpressionAny::ExpressionAny(const std::string& expression):
	ExpressionBase(expression)
{
}


ExpressionAny::ExpressionAny(CatRuntimeContext* compileContext, const std::string& expression):
	ExpressionBase(compileContext, expression)
{
	compile(compileContext);
}


const CatValue ExpressionAny::getValue(CatRuntimeContext* runtimeContext)
{
	if (isConstant)
	{
		return cachedValue;
	}
	else if (expressionAST != nullptr)
	{
		return expressionAST->execute(runtimeContext);
	}
	else
	{
		return CatValue();
	}
}


void ExpressionAny::compile(CatRuntimeContext* context)
{
	if (parse(context, CatGenericType()) && isConstant)
	{
		cachedValue = expressionAST->execute(context);
	}
}