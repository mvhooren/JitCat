/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatStatement.h"

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
	class CatAssignmentOperator;

	class CatVariableDeclaration: public CatStatement
	{
	public:
		CatVariableDeclaration(CatTypeNode* typeNode, const std::string& name, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme, CatTypedExpression* initialization = nullptr);
		CatVariableDeclaration(const CatVariableDeclaration& other);

		virtual ~CatVariableDeclaration();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;

		const std::string& getName() const;
		const CatTypeNode& getType() const;
		
	private:
		std::unique_ptr<CatTypeNode> type;
		std::string name;
		Tokenizer::Lexeme nameLexeme;

		std::unique_ptr<CatAssignmentOperator> initializationExpression;
		Reflection::TypeMemberInfo* memberInfo;
	};

}