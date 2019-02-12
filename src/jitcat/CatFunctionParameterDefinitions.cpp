#include "jitcat/CatFunctionParameterDefinitions.h"
#include "jitcat/CatVariableDeclaration.h"
#include "jitcat/CatLog.h"

using namespace jitcat;
using namespace jitcat::AST;


CatFunctionParameterDefinitions::CatFunctionParameterDefinitions(const std::vector<CatVariableDeclaration*>& parameterDeclarations)
{
	for (auto& iter : parameterDeclarations)
	{
		parameters.emplace_back(iter);
	}
}


CatFunctionParameterDefinitions::~CatFunctionParameterDefinitions()
{
}


void CatFunctionParameterDefinitions::print() const
{
	bool addComma = false;
	for (auto& iter: parameters)
	{
		if (addComma)
		{
			Tools::CatLog::log(", ");
		}
		addComma = true;
		iter->print();
	}
}


CatASTNodeType CatFunctionParameterDefinitions::getNodeType()
{
	return CatASTNodeType::FunctionParameterDefinitions;
}
