#pragma once


#include "jitcat/CatStatement.h"

#include <memory>
namespace jitcat::AST
{
	class CatTypedExpression;
	class CatScopeBlock;
	class CatASTNode;

	class CatIfStatement: public CatStatement
	{
	public:
		CatIfStatement(CatTypedExpression* condition, CatScopeBlock* ifBody, CatASTNode* elseNode = nullptr);

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

	private:
		std::unique_ptr<CatTypedExpression> condition;
		std::unique_ptr<CatScopeBlock> ifBody;
		//This is either a CatScopeBlock in case of an 'else' or another CatIfStatement in case of an 'else if' or nullptr if there is no else.
		std::unique_ptr<CatASTNode> elseNode;

	};
}