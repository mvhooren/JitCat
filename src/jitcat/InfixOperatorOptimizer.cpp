/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "InfixOperatorOptimizer.h"
#include "CatTypedExpression.h"
#include "CatLiteral.h"


CatTypedExpression* InfixOperatorOptimizer::tryCollapseInfixOperator(std::unique_ptr<CatTypedExpression>& lhs, 
																	 std::unique_ptr<CatTypedExpression>& rhs, 
																	 CatInfixOperatorType infixOperator)
{
	switch (infixOperator)
	{
		case CatInfixOperatorType::Multiply:	return tryCollapseMultiplication(lhs, rhs);		break;
		case CatInfixOperatorType::Divide:		return tryCollapseDivision(lhs, rhs);			break;
		case CatInfixOperatorType::Plus:		return tryCollapseAddition(lhs, rhs);			break;
		case CatInfixOperatorType::Minus:		return tryCollapseSubtraction(lhs, rhs);		break;
		case CatInfixOperatorType::LogicalAnd:	return tryCollapseLogicalAnd(lhs, rhs);			break;
		case CatInfixOperatorType::LogicalOr:	return tryCollapseLogicalOr(lhs, rhs);			break;
	}
	return nullptr;
}


CatTypedExpression* InfixOperatorOptimizer::tryCollapseMultiplication(std::unique_ptr<CatTypedExpression>& lhs, std::unique_ptr<CatTypedExpression>& rhs)
{
	if (typedExpressionEqualsConstant(lhs.get(), 1.0f))			return rhs.release();
	else if (typedExpressionEqualsConstant(rhs.get(), 1.0f))	return lhs.release();
	else if (typedExpressionEqualsConstant(lhs.get(), 0.0f))	return lhs.release();
	else if (typedExpressionEqualsConstant(rhs.get(), 0.0f))	return rhs.release();
	else														return nullptr;
}


CatTypedExpression* InfixOperatorOptimizer::tryCollapseAddition(std::unique_ptr<CatTypedExpression>& lhs, std::unique_ptr<CatTypedExpression>& rhs)
{
	if (lhs.get()->getType().isStringType() || rhs.get()->getType().isStringType())	return nullptr;
	else if (typedExpressionEqualsConstant(lhs.get(), 0.0f))						return rhs.release();
	else if (typedExpressionEqualsConstant(rhs.get(), 0.0f))						return lhs.release();
	else																			return nullptr;
}


CatTypedExpression* InfixOperatorOptimizer::tryCollapseSubtraction(std::unique_ptr<CatTypedExpression>& lhs, std::unique_ptr<CatTypedExpression>& rhs)
{
	if (typedExpressionEqualsConstant(rhs.get(), 0.0f))			return lhs.release();
	else														return nullptr;
}


CatTypedExpression* InfixOperatorOptimizer::tryCollapseDivision(std::unique_ptr<CatTypedExpression>& lhs, std::unique_ptr<CatTypedExpression>& rhs)
{
	if (typedExpressionEqualsConstant(rhs.get(), 1.0f))			return lhs.release();
	else if (typedExpressionEqualsConstant(lhs.get(), 0.0f))	return lhs.release();
	else														return nullptr;
}


CatTypedExpression* InfixOperatorOptimizer::tryCollapseLogicalAnd(std::unique_ptr<CatTypedExpression>& lhs, std::unique_ptr<CatTypedExpression>& rhs)
{
	if (typedExpressionEqualsConstant(lhs.get(), false))		return new CatLiteral(false);
	else														return nullptr;
}


CatTypedExpression* InfixOperatorOptimizer::tryCollapseLogicalOr(std::unique_ptr<CatTypedExpression>& lhs, std::unique_ptr<CatTypedExpression>& rhs)
{
	if (typedExpressionEqualsConstant(lhs.get(), true))			return new CatLiteral(true);
	else														return nullptr;
}


bool InfixOperatorOptimizer::typedExpressionEqualsConstant(CatTypedExpression* expression, float constant)
{
	if (expression->getNodeType() == CatASTNodeType::Literal)
	{
		CatLiteral* literalExpression = static_cast<CatLiteral*>(expression);
		return literalExpression->toFloatValue() == constant;
	}
	else
	{
		return false;
	}
}


bool InfixOperatorOptimizer::typedExpressionEqualsConstant(CatTypedExpression* expression, bool constant)
{
	if (expression->getNodeType() == CatASTNodeType::Literal)
	{
		CatLiteral* literalExpression = static_cast<CatLiteral*>(expression);
		return literalExpression->toBoolValue() == constant;
	}
	else
	{
		return false;
	}
}
