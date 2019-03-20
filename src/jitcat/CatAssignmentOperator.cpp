/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatAssignmentOperator.h"
#include "jitcat/AssignableType.h"
#include "jitcat/ASTHelper.h"
#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatLog.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Tools.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Reflection;
using namespace jitcat::Tools;


CatAssignmentOperator::CatAssignmentOperator(CatTypedExpression* lhs, CatTypedExpression* rhs):
	lhs(lhs),
	rhs(rhs)
{
}


void CatAssignmentOperator::print() const
{
	CatLog::log("(");
	lhs->print();
	CatLog::log(" = ");
	rhs->print();
	CatLog::log(")");
}


CatASTNodeType CatAssignmentOperator::getNodeType()
{
	return CatASTNodeType::AssignmentOperator;
}


std::any CatAssignmentOperator::execute(CatRuntimeContext* runtimeContext)
{
	CatAssignableExpression* lhsAssignable = static_cast<CatAssignableExpression*>(lhs.get());

	AssignableType assignableType = AssignableType::None;
	std::any target = lhsAssignable->executeAssignable(runtimeContext, assignableType);
	std::any source = rhs->execute(runtimeContext);
	ASTHelper::doAssignment(target, source, lhs->getType(), assignableType);

	return std::any();
}


bool CatAssignmentOperator::typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext)
{
	if (lhs->typeCheck(compiletimeContext, errorManager, errorContext) && rhs->typeCheck(compiletimeContext, errorManager, errorContext))
	{
		CatGenericType leftType = lhs->getType();
		CatGenericType rightType = rhs->getType();
		if (leftType != rightType
			&& leftType.isBasicType() && rightType.isBasicType())
		{
			//Automatic type conversion
			ASTHelper::doTypeConversion(this->rhs, lhs->getType());
		}
		if (!leftType.isWritable() || leftType.isConst() || !lhs->isAssignable())
		{
			errorManager->compiledWithError("Assignment failed because target cannot be assigned.", errorContext);
		}
		else if (leftType == rightType)
		{
			return true;
		}
		else
		{
			errorManager->compiledWithError(Tools::append("Cannot assign ", rightType.toString(), " to ", leftType.toString(), "."), errorContext);
		}
	}
	return false;
}


CatGenericType CatAssignmentOperator::getType() const
{
	return lhs->getType().toUnmodified();
}


bool CatAssignmentOperator::isConst() const
{
	return false;
}


CatTypedExpression* CatAssignmentOperator::constCollapse(CatRuntimeContext* compileTimeContext)
{
	ASTHelper::updatePointerIfChanged(lhs, lhs->constCollapse(compileTimeContext));
	ASTHelper::updatePointerIfChanged(rhs, rhs->constCollapse(compileTimeContext));
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
