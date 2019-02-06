#pragma once

#include "CatDefinition.h"
#include "CatGenericType.h"


namespace jitcat::AST
{

	class CatClassDefinition: public CatDefinition
	{
	public:
		CatClassDefinition(const std::string& name);
		virtual ~CatClassDefinition();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

	private:
		std::string name;
	};

};
