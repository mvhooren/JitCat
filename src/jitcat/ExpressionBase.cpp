/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ExpressionBase.h"
#include "CatArgumentList.h"
#include "CatFunctionCall.h"
#include "CatPrefixOperator.h"
#include "CatRuntimeContext.h"
#include "CatTypedExpression.h"
#include "ExpressionErrorManager.h"
#include "Document.h"
#include "JitCat.h"
#include "SLRParseResult.h"
#include "Tools.h"

#include <cassert>


ExpressionBase::ExpressionBase():
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
}


ExpressionBase::ExpressionBase(const char* expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
}


ExpressionBase::ExpressionBase(const std::string& expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
}


ExpressionBase::ExpressionBase(CatRuntimeContext* compileContext, const std::string& expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
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
		constCollapse(context);
		typeCheck(expectedType);
	}
	handleParseErrors(context);

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
		parseResult->errorMessage = valueType.getErrorMessage();
	}
	else 
	{	
		if (!expectedType.isUnknown())
		{
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
				if (expectedType.isScalarType() && valueType.isScalarType())
				{
					//Insert an automatic type conversion if the scalar types do not match.
					CatArgumentList* arguments = new CatArgumentList();
					arguments->arguments.emplace_back(static_cast<CatTypedExpression*>(parseResult->astRootNode));
					switch (expectedType.getCatType())
					{
						case CatType::Float:	expressionAST = new CatFunctionCall("toFloat", arguments);		break;
						case CatType::Int:		expressionAST = new CatFunctionCall("toInt", arguments);		break;
						default:				assert(false);	//Missing a conversion here?
					}
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
