/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatAssignmentOperator.h"
#include "AssignableType.h"
#include "CatAssignableExpression.h"
#include "CatLog.h"
#include "ASTHelper.h"
#include "Tools.h"


CatAssignmentOperator::CatAssignmentOperator(CatTypedExpression* lhs, CatTypedExpression* rhs):
	lhs(lhs),
	rhs(rhs)
{
	CatGenericType lhsType = lhs->getType();
	CatGenericType rhsType = rhs->getType();

	if (lhsType.isValidType() && rhsType.isValidType()
		&& lhsType != rhsType
		&& lhsType.isBasicType() && rhsType.isBasicType())
	{
		//Automatic type conversion
		ASTHelper::doTypeConversion(this->rhs, lhs->getType());
	}
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


CatGenericType CatAssignmentOperator::typeCheck()
{
	CatGenericType leftType = lhs->typeCheck();
	if (!leftType.isValidType())
	{
		return leftType;
	}
	CatGenericType rightType = rhs->typeCheck();
	if (!rightType.isValidType())
	{
		return rightType;
	}
	if (!leftType.isWritable() || leftType.isConst() || !lhs->isAssignable())
	{
		return CatGenericType(CatError("Assignment failed because target cannot be assigned."));
	}
	else if (leftType == rightType)
	{
		return leftType.toUnmodified();
	}
	else
	{
		return CatGenericType(CatError(Tools::append("Cannot assign ", rightType.toString(), " to ", leftType.toString(), ".")));
	}
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
