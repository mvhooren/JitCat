#pragma once

#include "jitcat/CatStatement.h"

#include <memory>
#include <vector>


namespace jitcat::AST
{
	class CatStatement;

	class CatScopeBlock: public CatStatement
	{
	public:
		CatScopeBlock(const std::vector<CatStatement*>& statementList);
		virtual ~CatScopeBlock();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

	private:
		std::vector<std::unique_ptr<CatStatement>> statements;
	};

}