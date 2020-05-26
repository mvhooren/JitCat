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
		CatIfStatement(CatTypedExpression* condition, CatScopeBlock* ifBody, const Tokenizer::Lexeme& lexeme, CatStatement* elseNode = nullptr);
		CatIfStatement(const CatIfStatement& other);

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual CatStatement* constCollapse(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual std::optional<bool> checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected) const override final;

	private:
		std::unique_ptr<CatTypedExpression> condition;
		std::unique_ptr<CatStatement> ifBody;
		//This is either a CatScopeBlock in case of an 'else' or another CatIfStatement in case of an 'else if' or nullptr if there is no else.
		std::unique_ptr<CatStatement> elseNode;
	};
}