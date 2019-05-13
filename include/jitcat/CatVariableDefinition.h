/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatDefinition.h"

#include <any>
#include <memory>
#include <string>


namespace jitcat::Reflection
{
	struct TypeMemberInfo;
}
namespace jitcat::AST
{

	class CatTypeNode;
	class CatTypedExpression;

	class CatVariableDefinition: public CatDefinition
	{
	public:
		CatVariableDefinition(CatTypeNode* typeNode, const std::string& name, const Tokenizer::Lexeme& lexeme, CatTypedExpression* initialization = nullptr);
		virtual ~CatVariableDefinition();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) override final;
		//virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		//virtual std::any execute(CatRuntimeContext* runtimeContext) override final;

		const std::string& getName() const;
		const CatTypeNode& getType() const;
		
		CatTypedExpression* releaseInitializationExpression();

	private:
		std::unique_ptr<CatTypeNode> type;
		std::string name;
		std::unique_ptr<CatTypedExpression> initializationExpression;
		Reflection::TypeMemberInfo* memberInfo;
	};

}