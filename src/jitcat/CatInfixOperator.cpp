/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatInfixOperator.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatStaticFunctionCall.h"
#include "jitcat/CatStaticScope.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/InfixOperatorOptimizer.h"
#include "jitcat/ASTHelper.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


CatInfixOperator::CatInfixOperator(CatTypedExpression* lhs, CatTypedExpression* rhs, CatInfixOperatorType operatorType, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& operatorLexeme):
	CatTypedExpression(lexeme),
	operatorLexeme(operatorLexeme),
	lhs(lhs),
	oper(operatorType),
	rhs(rhs),
	resultType(CatGenericType::unknownType)
{
}


jitcat::AST::CatInfixOperator::CatInfixOperator(const CatInfixOperator& other):
	CatTypedExpression(other),
	operatorLexeme(other.operatorLexeme),
	lhs(static_cast<CatTypedExpression*>(other.lhs->copy())),
	oper(other.oper),
	rhs(static_cast<CatTypedExpression*>(other.rhs->copy())),
	resultType(CatGenericType::unknownType)
{
}


CatASTNode* jitcat::AST::CatInfixOperator::copy() const
{
	return new CatInfixOperator(*this);
}


const CatGenericType& CatInfixOperator::getType() const
{
	if (overloadedOperator != nullptr)
	{
		return overloadedOperator->getType();
	}
	else
	{
		return resultType;
	}
}


bool CatInfixOperator::isConst() const 
{
	if (overloadedOperator != nullptr)
	{
		return overloadedOperator->isConst();
	}
	else
	{
		return lhs->isConst() && rhs->isConst();
	}
}


CatStatement* CatInfixOperator::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (overloadedOperator != nullptr)
	{
		ASTHelper::updatePointerIfChanged(overloadedOperator, overloadedOperator->constCollapse(compileTimeContext, errorManager, errorContext));
		return overloadedOperator.release();
	}

	ASTHelper::updatePointerIfChanged(lhs, lhs->constCollapse(compileTimeContext, errorManager, errorContext));
	ASTHelper::updatePointerIfChanged(rhs, rhs->constCollapse(compileTimeContext, errorManager, errorContext));

	bool lhsIsConst = lhs->isConst();
	bool rhsIsConst = rhs->isConst();
	if (   lhsIsConst 
		&& rhsIsConst 
		&& (lhs->getType().isBasicType() || lhs->getType().isStringValueType())
		&& (rhs->getType().isBasicType() || rhs->getType().isStringValueType()))
	{
		Tokenizer::Lexeme collapsedLexeme = InfixOperatorOptimizer::combineLexemes(lhs, rhs);
		const CatGenericType lhsType = lhs->getType();
		if (lhsType.isBasicType())
		{
			return new CatLiteral(calculateExpression(compileTimeContext), getType(), collapsedLexeme);
		}
	}
	else
	{
		CatTypedExpression* collapsedExpression = InfixOperatorOptimizer::tryCollapseInfixOperator(lhs, rhs, oper, compileTimeContext, errorManager, errorContext);
		if (collapsedExpression != nullptr)
		{
			return collapsedExpression;
		}
	}
	return this;
}


std::any CatInfixOperator::execute(CatRuntimeContext* runtimeContext)
{
	if (overloadedOperator != nullptr)
	{
		return overloadedOperator->execute(runtimeContext);
	}
	else
	{
		return calculateExpression(runtimeContext);
	}
}


bool CatInfixOperator::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (overloadedOperator != nullptr)
	{
		return overloadedOperator->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	if (lhs->typeCheck(compiletimeContext, errorManager, errorContext)
		&& rhs->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		CatGenericType leftType = lhs->getType();
		CatGenericType rightType = rhs->getType();
		InfixOperatorResultInfo resultInfo = leftType.getInfixOperatorResultInfo(oper, rightType);
		resultType = resultInfo.getResultType();
		if (resultType.isValidType())
		{
			if (resultInfo.getIsOverloaded())
			{
				if (!resultInfo.getIsStaticOverloaded())
				{
					std::vector<CatTypedExpression*> arguments = {rhs.release()};
					overloadedOperator = std::make_unique<CatMemberFunctionCall>(::toString(oper), operatorLexeme, lhs.release(), new CatArgumentList(arguments[0]->getLexeme(), arguments), getLexeme());
					return overloadedOperator->typeCheck(compiletimeContext, errorManager, errorContext);
				}
				else
				{
					std::vector<CatTypedExpression*> arguments = {lhs.release(), rhs.release()};
					overloadedOperator = std::make_unique<CatStaticFunctionCall>(new CatStaticScope(true, nullptr, resultInfo.getStaticOverloadedType()->getTypeName(), operatorLexeme, operatorLexeme), ::toString(oper), new CatArgumentList(arguments[0]->getLexeme(), arguments), getLexeme(),  operatorLexeme);
					return overloadedOperator->typeCheck(compiletimeContext, errorManager, errorContext);
				}
			}
			return true;
		}
		else if (resultInfo.getIsOverloaded())
		{
			errorManager->compiledWithError(Tools::append("Operator ", ::toString(oper), " not implemented for ", leftType.toString()), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Invalid operation: ", leftType.toString(), " ", ::toString(oper), " ", rightType.toString()), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}
	return false;
}


void CatInfixOperator::print() const
{
	CatLog::log("(");
	lhs->print();
	CatLog::log(" ");
	CatLog::log(toString(oper));
	CatLog::log(" ");
	rhs->print();
	CatLog::log(")");
}


CatTypedExpression* CatInfixOperator::getLeft() const
{
	return lhs.get();
}


 CatTypedExpression* CatInfixOperator::getRight() const
{
	return rhs.get();
}


CatInfixOperatorType CatInfixOperator::getOperatorType() const
{
	return oper;
}


inline std::any CatInfixOperator::calculateExpression(CatRuntimeContext* runtimeContext)
{
	//Optimization to prevent always calculating both sides of an && or an ||
	if (oper == CatInfixOperatorType::LogicalAnd || oper == CatInfixOperatorType::LogicalOr)
	{
		std::any lValue = lhs->execute(runtimeContext);
		//If lValue is true and oper is || then result is true
		//If lValue is false and oper is && then result is false
		if (std::any_cast<bool>(lValue) == (oper == CatInfixOperatorType::LogicalOr))
		{
			return lValue;
		}
		else
		{
			std::any rValue = rhs->execute(runtimeContext);
			if (oper == CatInfixOperatorType::LogicalAnd)
			{
				return std::any(std::any_cast<bool>(lValue) && std::any_cast<bool>(rValue));
			}
			else
			{
				return std::any(std::any_cast<bool>(lValue) || std::any_cast<bool>(rValue));
			}
		}
	}
	else
	{

		std::any lValue = lhs->execute(runtimeContext);
		std::any rValue = rhs->execute(runtimeContext);
		CatGenericType lType = lhs->getType();
		CatGenericType rType = rhs->getType();
		if (lType.isFloatType())
		{
			if		(rType.isIntType())			return calculateScalarExpression<float, int, float>(std::any_cast<float>(lValue), std::any_cast<int>(rValue));
			else if (rType.isFloatType())		return calculateScalarExpression<float, float, float>(std::any_cast<float>(lValue), std::any_cast<float>(rValue));
			else if (rType.isDoubleType())		return calculateScalarExpression<float, double, double>(std::any_cast<float>(lValue), std::any_cast<double>(rValue));
			else if (rType.isInt64Type())		return calculateScalarExpression<float, int64_t, float>(std::any_cast<float>(lValue), std::any_cast<int64_t>(rValue));
			else if (rType.isUInt64Type())		return calculateScalarExpression<float, uint64_t, float>(std::any_cast<float>(lValue), std::any_cast<uint64_t>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression<float, Configuration::CatString>(std::any_cast<float>(lValue), std::any_cast<Configuration::CatString>(rValue));
		}
		else if (lType.isDoubleType())
		{
			if		(rType.isIntType())			return calculateScalarExpression<double, int, double>(std::any_cast<double>(lValue), std::any_cast<int>(rValue));
			else if (rType.isFloatType())		return calculateScalarExpression<double, float, double>(std::any_cast<double>(lValue), std::any_cast<float>(rValue));
			else if (rType.isDoubleType())		return calculateScalarExpression<double, double, double>(std::any_cast<double>(lValue), std::any_cast<double>(rValue));
			else if (rType.isInt64Type())		return calculateScalarExpression<double, int64_t, double>(std::any_cast<double>(lValue), std::any_cast<int64_t>(rValue));
			else if (rType.isUInt64Type())		return calculateScalarExpression<double, uint64_t, double>(std::any_cast<double>(lValue), std::any_cast<uint64_t>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression<double, Configuration::CatString>(std::any_cast<double>(lValue), std::any_cast<Configuration::CatString>(rValue));
		}
		else if (lType.isIntType())
		{
			if		(rType.isIntType())			return calculateScalarExpression<int, int, int>(std::any_cast<int>(lValue), std::any_cast<int>(rValue));
			else if (rType.isFloatType())		return calculateScalarExpression<int, float, float>(std::any_cast<int>(lValue), std::any_cast<float>(rValue));
			else if (rType.isDoubleType())		return calculateScalarExpression<int, double, double>(std::any_cast<int>(lValue), std::any_cast<double>(rValue));
			else if (rType.isInt64Type())		return calculateScalarExpression<int, int64_t, int64_t>(std::any_cast<int>(lValue), std::any_cast<int64_t>(rValue));
			else if (rType.isUInt64Type())		return calculateScalarExpression<int, uint64_t, uint64_t>(std::any_cast<int>(lValue), std::any_cast<uint64_t>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression<int, Configuration::CatString>(std::any_cast<int>(lValue), std::any_cast<Configuration::CatString>(rValue));
		}
		else if (lType.isStringValueType())
		{
			if		(rType.isIntType())			return calculateStringExpression<Configuration::CatString, int>(std::any_cast<Configuration::CatString>(lValue), std::any_cast<int>(rValue));
			else if (rType.isFloatType())		return calculateStringExpression<Configuration::CatString, float>(std::any_cast<Configuration::CatString>(lValue), std::any_cast<float>(rValue));
			else if (rType.isDoubleType())		return calculateStringExpression<Configuration::CatString, double>(std::any_cast<Configuration::CatString>(lValue), std::any_cast<double>(rValue));
			else if (rType.isInt64Type())		return calculateStringExpression<Configuration::CatString, int64_t>(std::any_cast<Configuration::CatString>(lValue), std::any_cast<int64_t>(rValue));
			else if (rType.isUInt64Type())		return calculateStringExpression<Configuration::CatString, uint64_t>(std::any_cast<Configuration::CatString>(lValue), std::any_cast<uint64_t>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression(std::any_cast<Configuration::CatString>(lValue), std::any_cast<Configuration::CatString>(rValue));
			else if (rType.isBoolType())		return calculateStringExpression(std::any_cast<Configuration::CatString>(lValue), std::any_cast<bool>(rValue));
		}
		else if (lType.isInt64Type())
		{
			if		(rType.isIntType())			return calculateScalarExpression<int64_t, int, int64_t>(std::any_cast<int64_t>(lValue), std::any_cast<int>(rValue));
			else if (rType.isFloatType())		return calculateScalarExpression<int64_t, float, float>(std::any_cast<int64_t>(lValue), std::any_cast<float>(rValue));
			else if (rType.isDoubleType())		return calculateScalarExpression<int64_t, double, double>(std::any_cast<int64_t>(lValue), std::any_cast<double>(rValue));
			else if (rType.isInt64Type())		return calculateScalarExpression<int64_t, int64_t, int64_t>(std::any_cast<int64_t>(lValue), std::any_cast<int64_t>(rValue));
			else if (rType.isUInt64Type())		return calculateScalarExpression<int64_t, uint64_t, uint64_t>(std::any_cast<int64_t>(lValue), std::any_cast<uint64_t>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression<int64_t, Configuration::CatString>(std::any_cast<int64_t>(lValue), std::any_cast<Configuration::CatString>(rValue));
		}
		else if (lType.isUInt64Type())
		{
			if		(rType.isIntType())			return calculateScalarExpression<uint64_t, int, uint64_t>(std::any_cast<uint64_t>(lValue), std::any_cast<int>(rValue));
			else if (rType.isFloatType())		return calculateScalarExpression<uint64_t, float, float>(std::any_cast<uint64_t>(lValue), std::any_cast<float>(rValue));
			else if (rType.isDoubleType())		return calculateScalarExpression<uint64_t, double, double>(std::any_cast<uint64_t>(lValue), std::any_cast<double>(rValue));
			else if (rType.isInt64Type())		return calculateScalarExpression<uint64_t, int64_t, uint64_t>(std::any_cast<uint64_t>(lValue), std::any_cast<int64_t>(rValue));
			else if (rType.isUInt64Type())		return calculateScalarExpression<uint64_t, uint64_t, uint64_t>(std::any_cast<uint64_t>(lValue), std::any_cast<uint64_t>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression<uint64_t, Configuration::CatString>(std::any_cast<uint64_t>(lValue), std::any_cast<Configuration::CatString>(rValue));
		}
		else if (lType.isBoolType())
		{
			if		(rType.isBoolType())		return calculateBooleanExpression(std::any_cast<bool>(lValue), std::any_cast<bool>(rValue));
			else if (rType.isStringValueType())	return calculateStringExpression(std::any_cast<bool>(lValue), std::any_cast<Configuration::CatString>(rValue));
		}
		else if (lType.isPointerToReflectableObjectType() && rType.isPointerToReflectableObjectType())
		{
			switch (oper)
			{
				case CatInfixOperatorType::Equals:		return lType.getRawPointer(lValue) == rType.getRawPointer(rValue);
				case CatInfixOperatorType::NotEquals:	return lType.getRawPointer(lValue) != rType.getRawPointer(rValue);
				default:								assert(false);
			}
		}
		assert(false);
		return std::any();
	}
}