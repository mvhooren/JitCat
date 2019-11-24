/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/InfixOperatorOptimizer.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/CatLiteral.h"

using namespace jitcat::AST;

CatTypedExpression* InfixOperatorOptimizer::tryCollapseInfixOperator(std::unique_ptr<CatTypedExpression>& lhs, 
																	 std::unique_ptr<CatTypedExpression>& rhs, 
																	 CatInfixOperatorType infixOperator,
																	 jitcat::CatRuntimeContext* compileTimeContext)
{
	CatGenericType resultType = lhs->getType().getInfixOperatorResultType(infixOperator, rhs->getType());
	std::unique_ptr<CatTypedExpression> constCollapsed;
	switch (infixOperator)
	{
		case CatInfixOperatorType::Multiply:	constCollapsed.reset(tryCollapseMultiplication(lhs, rhs));	break;
		case CatInfixOperatorType::Divide:		constCollapsed.reset(tryCollapseDivision(lhs, rhs));		break;
		case CatInfixOperatorType::Plus:		constCollapsed.reset(tryCollapseAddition(lhs, rhs));		break;
		case CatInfixOperatorType::Minus:		constCollapsed.reset(tryCollapseSubtraction(lhs, rhs));		break;
		case CatInfixOperatorType::LogicalAnd:	constCollapsed.reset(tryCollapseLogicalAnd(lhs, rhs));		break;
		case CatInfixOperatorType::LogicalOr:	constCollapsed.reset(tryCollapseLogicalOr(lhs, rhs));		break;
		default: return nullptr;
	}
	if (constCollapsed.get() != nullptr)
	{
		if (resultType != constCollapsed->getType())
		{
			ASTHelper::doTypeConversion(constCollapsed, resultType);
			ASTHelper::updatePointerIfChanged(constCollapsed, constCollapsed->constCollapse(compileTimeContext));
		}
		return constCollapsed.release();
	}	
	else
	{
		return nullptr;
	}
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
		if (literalExpression->getType().isScalarType())
		{
			return CatGenericType::convertToFloat(literalExpression->getValue(), literalExpression->getType()) == constant;
		}
		else
		{
			return false;
		}
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
		if (literalExpression->getType().isBoolType())
		{
			return std::any_cast<bool>(literalExpression->getValue()) == constant;
		}
	}
	return false;
}
