#include "jitcat/CatReturnStatement.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatTypedExpression.h"

using namespace jitcat;
using namespace jitcat::AST;


CatReturnStatement::CatReturnStatement(CatTypedExpression* returnExpression):
	returnExpression(returnExpression)
{
}


CatReturnStatement::~CatReturnStatement()
{
}


void CatReturnStatement::print() const
{
	Tools::CatLog::log("return");
	if (returnExpression.get() != nullptr)
	{
		Tools::CatLog::log(" ");
		returnExpression->print();
	}
}


CatASTNodeType CatReturnStatement::getNodeType()
{
	return CatASTNodeType::ReturnStatement;
}


std::any CatReturnStatement::execute(CatRuntimeContext* runtimeContext)
{
	return std::any();
}


CatGenericType CatReturnStatement::typeCheck()
{
	return CatGenericType();
}


CatGenericType CatReturnStatement::getType() const
{
	return CatGenericType();
}


bool CatReturnStatement::isConst() const
{
	return false;
}


CatTypedExpression* CatReturnStatement::constCollapse(CatRuntimeContext* compileTimeContext)
{
	return this;
}
