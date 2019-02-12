#include "jitcat/CatIfStatement.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatScopeBlock.h"
#include "jitcat/CatTypedExpression.h"

using namespace jitcat;
using namespace jitcat::AST;


CatIfStatement::CatIfStatement(CatTypedExpression* condition, CatScopeBlock* ifBody, CatASTNode* elseNode):
	condition(condition),
	ifBody(ifBody),
	elseNode(elseNode)
{

}


void CatIfStatement::print() const
{
	Tools::CatLog::log("if (");
	condition->print();
	Tools::CatLog::log(")\n");
	ifBody->print();
	if (elseNode != nullptr)
	{
		Tools::CatLog::log("else");
		if (elseNode->getNodeType() == CatASTNodeType::ScopeBlock)
		{
			Tools::CatLog::log("\n");
		}
		elseNode->print();
	}
}


CatASTNodeType CatIfStatement::getNodeType()
{
	return CatASTNodeType::IfStatement;
}
