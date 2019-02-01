/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ExpressionBase.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatFunctionCall.h"
#include "jitcat/CatPrefixOperator.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Document.h"
#include "jitcat/JitCat.h"
#ifdef ENABLE_LLVM
#include "jitcat/LLVMCodeGenerator.h"
#include "jitcat/LLVMCompileTimeContext.h"
#endif
#include "jitcat/SLRParseResult.h"
#include "jitcat/Tools.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::LLVM;
using namespace jitcat::Parser;
using namespace jitcat::Tokenizer;


ExpressionBase::ExpressionBase(bool expectAssignable):
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr),
	expectAssignable(expectAssignable)
{
}


ExpressionBase::ExpressionBase(const char* expression, bool expectAssignable):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr),
	expectAssignable(expectAssignable)
{
}


ExpressionBase::ExpressionBase(const std::string& expression, bool expectAssignable):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr),
	expectAssignable(expectAssignable)
{
}


ExpressionBase::ExpressionBase(CatRuntimeContext* compileContext, const std::string& expression, bool expectAssignable):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr),
	expectAssignable(expectAssignable)
{
}


ExpressionBase::~ExpressionBase()
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->expressionDeleted(this);
	}
}


void ExpressionBase::setExpression(const std::string& expression_, CatRuntimeContext* compileContext)
{
	if (expression != expression_)
	{
		expression = expression_;
		if (compileContext != nullptr)
		{
			compile(compileContext);
		}
	}
}


const std::string& ExpressionBase::getExpression() const
{
	return expression;
}


bool ExpressionBase::isLiteral() const
{
	return expressionIsLiteral;
}


bool ExpressionBase::isConst() const
{
	return isConstant;
}


bool ExpressionBase::hasError() const
{
	return !(parseResult.get() != nullptr
		     && parseResult->success);
}


const CatGenericType ExpressionBase::getType() const
{
	return valueType;
}


bool ExpressionBase::parse(CatRuntimeContext* context, const CatGenericType& expectedType)
{
	if (context != nullptr)
	{
		errorManagerHandle = context->getErrorManager();
	}
	else
	{
		errorManagerHandle = nullptr;
	}

	isConstant = false;
	expressionIsLiteral = false;

	Document document(expression.c_str(), expression.length());
	parseResult.reset(JitCat::get()->parse(&document, context));

	if (parseResult->success)
	{
		expressionAST = static_cast<CatTypedExpression*>(parseResult->astRootNode);
		typeCheck(expectedType);
		if (parseResult->success)
		{
			constCollapse(context);
		}
	}
	handleParseErrors(context);
	//typeCheck may have changed parseResult->success
	if (parseResult->success && !isConstant)
	{
		compileToNativeCode(context);
	}

	return parseResult->success;
}


void ExpressionBase::constCollapse(CatRuntimeContext* context)
{
	bool isMinusPrefixWithLiteral = false;
	//If the expression is a minus prefix operator combined with a literal, then we need to count the whole expression as a literal.
	if (expressionAST->getNodeType() == CatASTNodeType::PrefixOperator)
	{
		CatPrefixOperator* prefixOp = static_cast<CatPrefixOperator*>(expressionAST);
		if (prefixOp->rhs != nullptr
			&& prefixOp->oper == CatPrefixOperator::Operator::Minus
			&& prefixOp->rhs->getNodeType() == CatASTNodeType::Literal)
		{
			isMinusPrefixWithLiteral = true;
		}
	}
	CatTypedExpression* newExpression = expressionAST->constCollapse(context);
	if (newExpression != expressionAST)
	{
		delete expressionAST;
		expressionAST = newExpression;
		parseResult->astRootNode = newExpression;
		expressionIsLiteral = isMinusPrefixWithLiteral;
	}
	else
	{
		expressionIsLiteral = expressionAST->getNodeType() == CatASTNodeType::Literal;
	}
}


void ExpressionBase::typeCheck(const CatGenericType& expectedType)
{
	valueType = expressionAST->typeCheck();

	if (!valueType.isValidType())
	{
		parseResult->success = false;
		parseResult->errorMessage = valueType.getError().getMessage();
	}
	else 
	{	
		if (!expectedType.isUnknown())
		{
			if (expectAssignable && !valueType.isWritable())
			{
				parseResult->success = false;
				parseResult->errorMessage = std::string(Tools::append("Expression result is read only. Expected a writable ", expectedType.toString(), "."));
			}
			if (expectedType.isObjectType())
			{
				const std::string typeName = expectedType.getObjectTypeName();
				if (!valueType.isObjectType())
				{
					parseResult->success = false;
					parseResult->errorMessage = Tools::append("Expected a ", typeName);
				}
				else if (valueType.getObjectTypeName() != typeName)
				{
					parseResult->success = false;
					parseResult->errorMessage = Tools::append("Expected a ", typeName, ", got a ", valueType.getObjectTypeName());
				}
			}
			else if (expectedType.isVoidType() && valueType.isVoidType())
			{
				parseResult->success = true;
				parseResult->astRootNode = expressionAST;
			}
			else if (valueType != expectedType)
			{
				if (expectedType.isVoidType())
				{
					//Insert an automatic type conversion to void.
					CatArgumentList* arguments = new CatArgumentList();
					arguments->arguments.emplace_back(static_cast<CatTypedExpression*>(parseResult->astRootNode));
					expressionAST = new CatFunctionCall("toVoid", arguments);
					parseResult->astRootNode = expressionAST;
					valueType = expressionAST->getType();
				}
				else if (expectedType.isScalarType() && valueType.isScalarType())
				{
					//Insert an automatic type conversion if the scalar types do not match.
					CatArgumentList* arguments = new CatArgumentList();
					arguments->arguments.emplace_back(static_cast<CatTypedExpression*>(parseResult->astRootNode));

					if		(expectedType.isFloatType())	expressionAST = new CatFunctionCall("toFloat", arguments);
					else if (expectedType.isIntType())		expressionAST = new CatFunctionCall("toInt", arguments);
					else assert(false);	//Missing a conversion here?
					
					parseResult->astRootNode = expressionAST;
					valueType = expressionAST->getType();
				}
				else
				{
					parseResult->success = false;
					parseResult->errorMessage = std::string(Tools::append("Expected a ", expectedType.toString()));
				}
			}
		}
		if (parseResult->success)
		{
			isConstant = expressionAST->isConst();
		}
	}
}


void ExpressionBase::handleParseErrors(CatRuntimeContext* context)
{
	if (!parseResult->success)
	{
		if (context != nullptr)
		{
			ExpressionErrorManager* errorManager = context->getErrorManager();
			if (errorManager != nullptr)
			{
				std::string contextName = "";
				if (context != nullptr)
				{
					contextName = context->getContextName().c_str();
				}
				std::string errorMessage;
				if (contextName != "")
				{
					errorMessage = Tools::append("ERROR in ", contextName, ": \n", expression ,"\n", parseResult->errorMessage);
				}
				else
				{
					errorMessage = Tools::append("ERROR: \n", expression ,"\n", parseResult->errorMessage);
				}
				if (parseResult->errorPosition >= 0)
				{
					errorMessage = Tools::append(errorMessage, " Offset: ", parseResult->errorPosition);
				}
				errorManager->compiledWithError(errorMessage, this);
			}
		}
		expressionIsLiteral = false;
		expressionAST = nullptr;
	}
	else if (context != nullptr)
	{
		ExpressionErrorManager* errorManager = context->getErrorManager();
		if (errorManager != nullptr)
		{
			errorManager->compiledWithoutErrors(this);
		}
	}
}


void ExpressionBase::compileToNativeCode(CatRuntimeContext* context)
{
#ifdef ENABLE_LLVM
	if (context != nullptr)
	{
		if (!isConstant)
		{
			LLVMCompileTimeContext llvmCompileContext(context);
			llvmCompileContext.options.enableDereferenceNullChecks = true;
			intptr_t functionAddress = 0;
			codeGenerator = context->getCodeGenerator();
			if (!expectAssignable)
			{
				functionAddress = codeGenerator->generateAndGetFunctionAddress(expressionAST, &llvmCompileContext);
			}
			else if (expressionAST->isAssignable())
			{
				functionAddress = codeGenerator->generateAndGetAssignFunctionAddress(static_cast<CatAssignableExpression*>(expressionAST), &llvmCompileContext);
			}
			if (functionAddress != 0)
			{
				handleCompiledFunction(functionAddress);
			}
			else
			{
				assert(false);
			}
		}
	}
#endif //ENABLE_LLVM
}
