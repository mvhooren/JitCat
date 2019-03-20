/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatStatement.h"

#include <memory>
#include <string>


namespace jitcat::AST
{

	class CatTypeNode;
	class CatTypedExpression;

	class CatVariableDeclaration: public CatStatement
	{
	public:
		CatVariableDeclaration(CatTypeNode* typeNode, const std::string& name, CatTypedExpression* initialization = nullptr);
		virtual ~CatVariableDeclaration();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		
		const std::string& getName() const;
		const CatTypeNode& getType() const;
		
	private:
		std::unique_ptr<CatTypeNode> type;
		std::string name;
		std::unique_ptr<CatTypedExpression> initializationExpression;
	};

}