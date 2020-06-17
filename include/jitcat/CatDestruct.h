/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/CatStatement.h"

#include <memory>


namespace jitcat::AST
{
	class CatAssignableExpression;
	class CatArgumentList;
	class CatIdentifier;
	//Default constructs the target variable
	class CatDestruct: public CatStatement
	{
	public:
		CatDestruct(const Tokenizer::Lexeme& lexeme, std::unique_ptr<CatIdentifier> identifier);
		CatDestruct(const CatDestruct& other);
		virtual ~CatDestruct();

		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual CatStatement* constCollapse(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual std::any execute(jitcat::CatRuntimeContext* runtimeContext) override final;
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		const CatGenericType& getType() const;

		CatAssignableExpression* getAssignable() const;

	private:
		CatGenericType assignableType;
		std::unique_ptr<CatAssignableExpression> assignable;

		std::unique_ptr<CatStatement> destructorStatement;
	};

}