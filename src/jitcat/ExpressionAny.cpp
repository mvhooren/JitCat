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


ExpressionAny::ExpressionAny():
	expressionIsLiteral(false),
	expressionAST(nullptr),
	parseResult(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
}


ExpressionAny::ExpressionAny(const ExpressionAny& other):
	expression(other.expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	parseResult(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
}


ExpressionAny::ExpressionAny(const char* expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	parseResult(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
}


ExpressionAny::ExpressionAny(const std::string& expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	parseResult(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
}


ExpressionAny::ExpressionAny(CatRuntimeContext* compileContext, const std::string& expression):
	expression(expression),
	expressionIsLiteral(false),
	expressionAST(nullptr),
	parseResult(nullptr),
	isConstant(false),
	errorManagerHandle(nullptr)
{
	compile(compileContext);
}


ExpressionAny::~ExpressionAny()
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->expressionDeleted(this);
	}
	delete parseResult;
}


void ExpressionAny::setExpression(const std::string& expression_, CatRuntimeContext* compileContext)
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


const std::string& ExpressionAny::getExpression() const
{
	return expression;
}


bool ExpressionAny::isLiteral() const
{
	return expressionIsLiteral;
}


bool ExpressionAny::isConst() const
{
	return isConstant;
}


bool ExpressionAny::hasError() const
{
	return !(parseResult != nullptr
			&& parseResult->success);
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
	errorManagerHandle = context->getErrorManager();
	isConstant = false;
	delete parseResult;
	Document document(expression.c_str(), expression.length());
	parseResult = JitCat::get()->parse(&document, context);
	if (parseResult->success)
	{
		CatTypedExpression* typedExpression = static_cast<CatTypedExpression*>(parseResult->astRootNode);
		bool isMinusPrefixWithLiteral = false;
		//If the expression is a minus prefix operator combined with a literal, then we need to count the whole expression as a literal.
		if (typedExpression->getNodeType() == CatASTNodeType::PrefixOperator)
		{
			CatPrefixOperator* prefixOp = static_cast<CatPrefixOperator*>(typedExpression);
			if (prefixOp->rhs != nullptr
				&& prefixOp->oper == CatPrefixOperator::Operator::Minus
				&& prefixOp->rhs->getNodeType() == CatASTNodeType::Literal)
			{
				isMinusPrefixWithLiteral = true;
			}
		}
		CatTypedExpression* newExpression = typedExpression->constCollapse(context);
		if (newExpression != typedExpression)
		{
			delete typedExpression;
			typedExpression = newExpression;
			parseResult->astRootNode = newExpression;
			expressionIsLiteral = isMinusPrefixWithLiteral;
		}
		else
		{
			expressionIsLiteral = typedExpression->getNodeType() == CatASTNodeType::Literal;
		}
		expressionAST = typedExpression;
		//Type check by just executing the expression and checking the result.
		//The compile-time context should give correct default values for type checking
		valueType = expressionAST->typeCheck();
		if (!valueType.isValidType())
		{
			parseResult->success = false;
			parseResult->errorMessage = valueType.getErrorMessage();
		}
	}
	if (!parseResult->success)
	{
		std::string contextName = context->getContextName().c_str();
		std::string errorMessage;
		if (contextName != "")
		{
			errorMessage = Tools::append("ERROR in ", contextName, ": \n", expression, "\n", parseResult->errorMessage);
		}
		else
		{
			errorMessage = Tools::append("ERROR: \n", expression, "\n", parseResult->errorMessage);
		}
		errorMessage = Tools::append(errorMessage, " Offset: ", parseResult->errorPosition);

		context->getErrorManager()->compiledWithError(errorMessage, this);
		expressionIsLiteral = false;
		expressionAST = nullptr;
		parseResult->astRootNode = nullptr;//QQQ leaking
	}
	else
	{
		if (expressionAST->isConst())
		{
			isConstant = true;
			cachedValue = expressionAST->execute(context);
		}
		context->getErrorManager()->compiledWithoutErrors(this);
	}
}


const CatGenericType ExpressionAny::getType() const
{
	return valueType;
}


std::string& ExpressionAny::getExpressionForSerialisation()
{
	return expression;
}
