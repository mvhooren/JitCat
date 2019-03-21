/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

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
		CatIfStatement(CatTypedExpression* condition, CatScopeBlock* ifBody, const Tokenizer::Lexeme& lexeme, CatASTNode* elseNode = nullptr);

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final { return std::any();}
	private:
		std::unique_ptr<CatTypedExpression> condition;
		std::unique_ptr<CatScopeBlock> ifBody;
		//This is either a CatScopeBlock in case of an 'else' or another CatIfStatement in case of an 'else if' or nullptr if there is no else.
		std::unique_ptr<CatASTNode> elseNode;

	};
}