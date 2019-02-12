#pragma once

#include "jitcat/CatASTNode.h"

#include <memory>
#include <vector>


namespace jitcat::AST
{
	class CatVariableDeclaration;

	class CatFunctionParameterDefinitions: public CatASTNode
	{
	public:
		CatFunctionParameterDefinitions(const std::vector<CatVariableDeclaration*>& parameterDeclarations);
		virtual ~CatFunctionParameterDefinitions();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

	private:
		std::vector<std::unique_ptr<CatVariableDeclaration>> parameters;
	};

}
