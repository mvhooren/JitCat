/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/InfixOperatorOptimizer.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/CatLiteral.h"

using namespace jitcat;
using namespace jitcat::AST;

CatTypedExpression* InfixOperatorOptimizer::tryCollapseInfixOperator(std::unique_ptr<CatTypedExpression>& lhs, 
																	 std::unique_ptr<CatTypedExpression>& rhs, 
																	 CatInfixOperatorType infixOperator,
																	 CatRuntimeContext* compileTimeContext, 
																	 ExpressionErrorManager* errorManager, 
																	 void* errorContext)
{
	CatGenericType resultType = lhs->getType();
	if ((lhs->getType().isFloatType() && rhs->getType().isIntType())
		|| (rhs->getType().isFloatType() && lhs->getType().isIntType()))
	{
		resultType = CatGenericType::floatType;
	}
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
			constCollapsed->typeCheck(compileTimeContext, errorManager, errorContext);
			ASTHelper::updatePointerIfChanged(constCollapsed, constCollapsed->constCollapse(compileTimeContext, errorManager, errorContext));
		}
		return constCollapsed.release();
	}	
	else
	{
		return nullptr;
	}
}


jitcat::Tokenizer::Lexeme jitcat::AST::InfixOperatorOptimizer::combineLexemes(std::unique_ptr<CatTypedExpression>& lhs, std::unique_ptr<CatTypedExpression>& rhs)
{
	Tokenizer::Lexeme lhsLexeme = lhs->getLexeme();
	Tokenizer::Lexeme rhsLexeme = rhs->getLexeme();
	return Tokenizer::Lexeme(lhsLexeme.data(), rhsLexeme.data() + rhsLexeme.length() - lhsLexeme.data());
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
	if (lhs->getType().isStringType() || rhs->getType().isStringType())	return nullptr;
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
	if (typedExpressionEqualsConstant(lhs.get(), false))		return new CatLiteral(false, combineLexemes(lhs, rhs));
	else														return nullptr;
}


CatTypedExpression* InfixOperatorOptimizer::tryCollapseLogicalOr(std::unique_ptr<CatTypedExpression>& lhs, std::unique_ptr<CatTypedExpression>& rhs)
{
	if (typedExpressionEqualsConstant(lhs.get(), true))			return new CatLiteral(true, combineLexemes(lhs, rhs));
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
