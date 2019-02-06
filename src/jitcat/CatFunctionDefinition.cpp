#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatTypeNode.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


jitcat::AST::CatFunctionDefinition::CatFunctionDefinition(CatTypeNode* type, const std::string& name):
	type(type),
	name(name)
{
}


jitcat::AST::CatFunctionDefinition::~CatFunctionDefinition()
{
}


void jitcat::AST::CatFunctionDefinition::print() const
{
	type->print();
	CatLog::log(" ", name, "(){}");
}


CatASTNodeType jitcat::AST::CatFunctionDefinition::getNodeType()
{
	return CatASTNodeType::FunctionDefinition;
}
