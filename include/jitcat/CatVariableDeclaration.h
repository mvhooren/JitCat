#pragma once

#include "jitcat/CatStatement.h"

#include <memory>
#include <string>


namespace jitcat::AST
{

	class CatTypeNode;
	class CatTypedExpression;

	class CatVariableDeclaration: public CatStatement
	{
	public:
		CatVariableDeclaration(CatTypeNode* typeNode, const std::string& name, CatTypedExpression* initialization = nullptr);
		virtual ~CatVariableDeclaration();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

	private:
		std::unique_ptr<CatTypeNode> type;
		std::string name;
		std::unique_ptr<CatTypedExpression> initializationExpression;
	};

}