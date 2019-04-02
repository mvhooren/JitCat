/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include "jitcat/CatDefinition.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatScopeID.h"
#include "jitcat/ReflectableHandle.h"

#include <any>
#include <memory>

namespace jitcat::Reflection
{
	class CustomTypeInstance;
	class CustomTypeInfo;
}

namespace jitcat::AST
{
	class CatTypeNode;
	class CatFunctionParameterDefinitions;
	class CatScopeBlock;

	class CatFunctionDefinition: public CatDefinition
	{
	public:
		CatFunctionDefinition(CatTypeNode* type, const std::string& name, CatFunctionParameterDefinitions* parameters, CatScopeBlock* scopeBlock, const Tokenizer::Lexeme& lexeme);
		virtual ~CatFunctionDefinition();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) override final;
		std::any executeFunction(CatRuntimeContext* runtimeContext, Reflection::CustomTypeInstance* parameterValues);
		jitcat::Reflection::CustomTypeInfo* getParametersType() const;
		CatTypeNode* getReturnTypeNode() const;
		int getNumParameters() const;
		const std::string& getFunctionName() const;

	private:
		std::string name;
		std::unique_ptr<CatTypeNode> type;
		std::unique_ptr<CatFunctionParameterDefinitions> parameters;
		CatScopeID parametersScopeId;
		std::unique_ptr<CatScopeBlock> scopeBlock;
		Reflection::ReflectableHandle errorManagerHandle;
	};


};
