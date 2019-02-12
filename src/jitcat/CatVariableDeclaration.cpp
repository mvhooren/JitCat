#include "jitcat/CatVariableDeclaration.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/CatLog.h"

using namespace jitcat;
using namespace jitcat::AST;


CatVariableDeclaration::CatVariableDeclaration(CatTypeNode* typeNode, const std::string& name, CatTypedExpression* initialization):
	type(typeNode),
	name(name),
	initializationExpression(initialization)
{
}


CatVariableDeclaration::~CatVariableDeclaration()
{
}


void CatVariableDeclaration::print() const
{
	type->print();
	Tools::CatLog::log(" ", name);
	if (initializationExpression != nullptr)
	{
		Tools::CatLog::log(" = ");
		initializationExpression->print();
	}
}


CatASTNodeType CatVariableDeclaration::getNodeType()
{
	return CatASTNodeType::VariableDeclaration;
}
