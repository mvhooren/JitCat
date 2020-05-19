/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatTypedExpression.h"
#include "jitcat/LLVMForwardDeclares.h"


namespace jitcat::LLVM
{
	//This expression can be used at code-generation time to store a pre-generated llvm::Value* and pass it as a CatTypedExpression.
	//It should never be used outside of code generation.
	class LLVMPreGeneratedExpression: public AST::CatTypedExpression
	{
	public:
		LLVMPreGeneratedExpression(llvm::Value* value, CatGenericType type);

		virtual CatASTNode* copy() const override final;
		virtual const CatGenericType& getType() const override final;
		virtual void print() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual AST::CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		llvm::Value* getValue() const;

	private:
		CatGenericType type;
		llvm::Value* value;
	};
}