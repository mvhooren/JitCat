/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatAssignmentOperator.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatArgumentList.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatMemberFunctionCall.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/Tools.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatAssignmentOperator::CatAssignmentOperator(CatTypedExpression* lhs, CatTypedExpression* rhs, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& operatorLexeme):
	CatTypedExpression(lexeme),
	lhs(lhs),
	rhs(rhs),
	operatorLexeme(operatorLexeme)
{
}


jitcat::AST::CatAssignmentOperator::CatAssignmentOperator(const CatAssignmentOperator& other):
	CatTypedExpression(other),
	lhs(static_cast<CatTypedExpression*>(other.getLhs()->copy())),
	rhs(static_cast<CatTypedExpression*>(other.getRhs()->copy()))
{
}


CatASTNode* jitcat::AST::CatAssignmentOperator::copy() const
{
	return new CatAssignmentOperator(*this);
}


void CatAssignmentOperator::print() const
{
	CatLog::log("(");
	lhs->print();
	CatLog::log(" = ");
	rhs->print();
	CatLog::log(")");
}


CatASTNodeType CatAssignmentOperator::getNodeType() const
{
	return CatASTNodeType::AssignmentOperator;
}


std::any CatAssignmentOperator::execute(CatRuntimeContext* runtimeContext)
{
	if (operatorFunction != nullptr)
	{
		return operatorFunction->execute(runtimeContext);
	}
	else
	{
		CatAssignableExpression* lhsAssignable = static_cast<CatAssignableExpression*>(lhs.get());

		return ASTHelper::doAssignment(lhsAssignable, rhs.get(), runtimeContext);
	}
}


bool CatAssignmentOperator::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	type = CatGenericType::unknownType;
	if (operatorFunction != nullptr)
	{
		return operatorFunction->typeCheck(compiletimeContext, errorManager, errorContext);
	}
	else if (lhs->typeCheck(compiletimeContext, errorManager, errorContext) && rhs->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		if (lhs.get()->isAssignable())
		{
			//Do indirection conversion if needed.
			const CatGenericType& assignableType = static_cast<CatAssignableExpression*>( lhs.get())->getAssignableType();
			int expectedIndirection = 0;
			assignableType.removeIndirection(expectedIndirection);
			assert(expectedIndirection > 0);
			//this is the expected indirection;
			expectedIndirection--;

			int rhsIndirection = 0;
			rhs->getType().removeIndirection(rhsIndirection);
			
			CatGenericType expectedType = rhs->getType();

			while (rhsIndirection > expectedIndirection && expectedIndirection > 0)
			{
				rhsIndirection--;
				expectedType = *expectedType.getPointeeType();
			}

			//CatGenericType expectedType = 
			IndirectionConversionMode conversionMode = IndirectionConversionMode::None;
			bool indirectionConversionSuccess = ASTHelper::doIndirectionConversion(rhs, expectedType, false, conversionMode);
			//To silence unused variable warning in release builds.
			(void)indirectionConversionSuccess;
			assert(indirectionConversionSuccess);
		}

		CatGenericType leftType = lhs->getType();
		CatGenericType rightType = rhs->getType();
		if (leftType != rightType
			&& (leftType.isBasicType() || leftType.isStringType())
			&& (rightType.isBasicType() || rightType.isStringType()))
		{
			//Automatic type conversion
			if (ASTHelper::doTypeConversion(this->rhs, lhs->getType()))
			{
				rhs->typeCheck(compiletimeContext, errorManager, errorContext);
			}
		}
		if (!ASTHelper::checkAssignment(lhs.get(), rhs.get(), errorManager, compiletimeContext, errorContext, getLexeme()))
		{
			return false;
		}
		type = lhs->getType().toUnmodified();
		if (lhs->getType().isPointerToReflectableObjectType()
			&& lhs->getType().getOwnershipSemantics() == TypeOwnershipSemantics::Value)
		{
			 std::vector<CatTypedExpression*> arguments = {rhs.release()};
			 operatorFunction = std::make_unique<CatMemberFunctionCall>("=", operatorLexeme, lhs.release(), new CatArgumentList(arguments[0]->getLexeme(), arguments), getLexeme());
			 return operatorFunction->typeCheck(compiletimeContext, errorManager, errorContext);
		}
		return true;
	}
	return false;
}


const CatGenericType& CatAssignmentOperator::getType() const
{
	if (operatorFunction != nullptr)
	{
		return operatorFunction->getType();
	}
	return type;
}


bool CatAssignmentOperator::isConst() const
{
	return false;
}


CatStatement* CatAssignmentOperator::constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (operatorFunction != nullptr)
	{
		ASTHelper::updatePointerIfChanged(operatorFunction, operatorFunction->constCollapse(compileTimeContext, errorManager, errorContext));
		return operatorFunction.release();
	}
	ASTHelper::updatePointerIfChanged(lhs, lhs->constCollapse(compileTimeContext, errorManager, errorContext));
	ASTHelper::updatePointerIfChanged(rhs, rhs->constCollapse(compileTimeContext, errorManager, errorContext));
	return this;
}


CatTypedExpression* CatAssignmentOperator::getLhs() const
{
	return lhs.get();
}


CatTypedExpression* CatAssignmentOperator::getRhs() const
{
	return rhs.get();
}
