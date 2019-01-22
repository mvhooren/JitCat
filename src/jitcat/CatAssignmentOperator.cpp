#include "CatAssignmentOperator.h"
#include "OptimizationHelper.h"


CatAssignmentOperator::CatAssignmentOperator(CatTypedExpression* lhs, CatTypedExpression* rhs):
	lhs(lhs),
	rhs(rhs)
{
}


void CatAssignmentOperator::print() const
{
}


CatASTNodeType CatAssignmentOperator::getNodeType()
{
	return CatASTNodeType::AssignmentOperator;
}


std::any CatAssignmentOperator::execute(CatRuntimeContext* runtimeContext)
{
	return std::any();
}


CatGenericType CatAssignmentOperator::typeCheck()
{
	return CatGenericType::voidType;
}


CatGenericType CatAssignmentOperator::getType() const
{
	return CatGenericType::voidType;
}


bool CatAssignmentOperator::isConst() const
{
	return false;
}


CatTypedExpression* CatAssignmentOperator::constCollapse(CatRuntimeContext* compileTimeContext)
{
	OptimizationHelper::updatePointerIfChanged(lhs, lhs->constCollapse(compileTimeContext));
	OptimizationHelper::updatePointerIfChanged(rhs, rhs->constCollapse(compileTimeContext));
	return this;
}
