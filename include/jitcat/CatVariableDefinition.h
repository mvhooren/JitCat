/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatDefinition.h"
#include "jitcat/MemberVisibility.h"

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
		CatVariableDefinition(const CatVariableDefinition& other);

		virtual ~CatVariableDefinition();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) override final;

		const std::string& getName() const;
		const CatTypeNode& getType() const;

		const CatTypedExpression* getInitializationExpression() const;

		Reflection::MemberVisibility getVariableVisibility() const;
		void setVariableVisibility(Reflection::MemberVisibility variableVisibility);

	private:
		std::unique_ptr<CatTypeNode> type;
		std::string name;
		Reflection::MemberVisibility visibility;
		std::unique_ptr<CatTypedExpression> initializationExpression;

		Reflection::TypeMemberInfo* memberInfo;
	};

}