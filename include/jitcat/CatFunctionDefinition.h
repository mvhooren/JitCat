/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include "jitcat/CatDefinition.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/ReflectableHandle.h"

#include <memory>


namespace jitcat::AST
{
	class CatTypeNode;
	class CatFunctionParameterDefinitions;
	class CatScopeBlock;

	class CatFunctionDefinition: public CatDefinition
	{
	public:
		CatFunctionDefinition(CatTypeNode* type, const std::string& name, CatFunctionParameterDefinitions* parameters, CatScopeBlock* scopeBlock);
		virtual ~CatFunctionDefinition();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) override final;

	private:
		std::string name;
		std::unique_ptr<CatTypeNode> type;
		std::unique_ptr<CatFunctionParameterDefinitions> parameters;
		std::unique_ptr<CatScopeBlock> scopeBlock;
		Reflection::ReflectableHandle errorManagerHandle;
	};


};
