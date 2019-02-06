#pragma once


#include "CatDefinition.h"
#include "CatGenericType.h"

#include <memory>


namespace jitcat::AST
{
	class CatTypeNode;


	class CatFunctionDefinition: public CatDefinition
	{
	public:
		CatFunctionDefinition(CatTypeNode* type, const std::string& name);
		virtual ~CatFunctionDefinition();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

	private:
		std::string name;
		std::unique_ptr<CatTypeNode> type;
	};

};
