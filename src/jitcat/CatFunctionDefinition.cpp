#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatFunctionParameterDefinitions.h"
#include "jitcat/CatLog.h"
#include "jitcat/CatTypeNode.h"
#include "jitcat/CatScopeBlock.h"

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


jitcat::AST::CatFunctionDefinition::CatFunctionDefinition(CatTypeNode* type, const std::string& name, CatFunctionParameterDefinitions* parameters, CatScopeBlock* scopeBlock):
	type(type),
	name(name),
	parameters(parameters),
	scopeBlock(scopeBlock)
{
}


jitcat::AST::CatFunctionDefinition::~CatFunctionDefinition()
{
}


void jitcat::AST::CatFunctionDefinition::print() const
{
	type->print();
	CatLog::log(" ", name, "(");
	parameters->print();
	CatLog::log(")");
	scopeBlock->print();
}


CatASTNodeType jitcat::AST::CatFunctionDefinition::getNodeType()
{
	return CatASTNodeType::FunctionDefinition;
}
