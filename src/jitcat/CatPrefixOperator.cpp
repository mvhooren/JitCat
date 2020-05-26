/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatPrefixOperator.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatLiteral.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;

const char* CatPrefixOperator::conversionTable[] = {"!", "-"};


jitcat::AST::CatPrefixOperator::CatPrefixOperator(const Tokenizer::Lexeme& lexeme, Operator oper, CatTypedExpression* rhs):
	CatTypedExpression(lexeme), 
	resultType(CatGenericType::unknownType),
	oper(oper),
	rhs(rhs)
{
}


jitcat::AST::CatPrefixOperator::CatPrefixOperator(const CatPrefixOperator& other):
	CatTypedExpression(other),
	resultType(CatGenericType::unknownType),
	oper(other.oper),
	rhs(static_cast<CatTypedExpression*>(other.rhs->copy()))
{
}


CatASTNode* jitcat::AST::CatPrefixOperator::copy() const
{
	return new CatPrefixOperator(*this);
}


const CatGenericType& CatPrefixOperator::getType() const
{
	return resultType;
}


bool CatPrefixOperator::isConst() const 
{
	return rhs->isConst();
}


CatStatement* CatPrefixOperator::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	ASTHelper::updatePointerIfChanged(rhs, rhs->constCollapse(compileTimeContext, errorManager, errorContext));
	if (rhs->isConst())
	{
		return new CatLiteral(calculateExpression(compileTimeContext), getType(), rhs->getLexeme());
	}
	return this;
}


std::any CatPrefixOperator::execute(CatRuntimeContext* runtimeContext)
{
	return calculateExpression(runtimeContext);
}


bool CatPrefixOperator::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (rhs->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		CatGenericType rightType = rhs->getType();
		if (rightType.isBoolType()
			&& oper == Operator::Not)
		{
			resultType = CatGenericType::boolType;
			return true;
		}
		else if (rightType.isFloatType()
					&& oper == Operator::Minus)
		{
			resultType =  CatGenericType::floatType;
			return true;
		}
		else if (rightType.isDoubleType()
					&& oper == Operator::Minus)
		{
			resultType = CatGenericType::doubleType;
			return true;
		}
		else if (rightType.isIntType()
					&& oper == Operator::Minus)
		{
			resultType =  CatGenericType::intType;
			return true;
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Error: invalid operation: ", conversionTable[(unsigned int)oper], rightType.toString()), errorContext, compiletimeContext->getContextName(), getLexeme());
			return false;
		}
	}
	return false;
}


void CatPrefixOperator::print() const
{
	CatLog::log("(");
	CatLog::log(conversionTable[(unsigned int)oper]);
	rhs->print();
	CatLog::log(")");
}


const CatTypedExpression* jitcat::AST::CatPrefixOperator::getRHS() const
{
	return rhs.get();
}


CatPrefixOperator::Operator jitcat::AST::CatPrefixOperator::getOperator() const
{
	return oper;
}


inline std::any CatPrefixOperator::calculateExpression(CatRuntimeContext* runtimeContext)
{
	std::any rValue = rhs->execute(runtimeContext);
	if (rhs->getType().isBoolType()
		&& oper == Operator::Not)
	{
		return std::any(!std::any_cast<bool>(rValue));
	}
	else if (rhs->getType().isFloatType()
				&& oper == Operator::Minus)
	{
		return std::any(-std::any_cast<float>(rValue));
	}
	else if (rhs->getType().isDoubleType()
				&& oper == Operator::Minus)
	{
		return std::any(-std::any_cast<double>(rValue));
	}
	else if (rhs->getType().isIntType()
				&& oper == Operator::Minus)
	{
		return std::any(-std::any_cast<int>(rValue));
	}
	assert(false);
	return std::any();
}