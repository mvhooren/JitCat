/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatAssignableExpression.h"
#include "jitcat/CatGenericType.h"

#include <memory>

namespace jitcat::Reflection
{
	struct MemberFunctionInfo;
}

namespace jitcat::AST
{

	class CatArrayIndex: public CatAssignableExpression
	{
	public:
		CatArrayIndex(CatTypedExpression* base, CatTypedExpression* arrayIndex, const Tokenizer::Lexeme& lexeme);
		CatArrayIndex(const CatArrayIndex& other);

		//From CatTypedExpression:
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		//From CatAssignableExpression
		virtual bool isAssignable() const override final;
		virtual const CatGenericType& getAssignableType() const override final;
		virtual std::any executeAssignable(CatRuntimeContext* runtimeContext) override final;

		CatTypedExpression* getBase() const;
		CatTypedExpression* getIndex() const;
		bool isReflectedArray() const;
		jitcat::Reflection::MemberFunctionInfo* getArrayIndexOperatorFunction() const;

	private:
		bool typeCheckIntrinsicArray(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext);
		bool typeCheckOperatorArray(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext);

	private:
		CatGenericType arrayType;
		std::unique_ptr<CatTypedExpression> array;
		CatGenericType indexType;
		std::unique_ptr<CatTypedExpression> index;
		CatGenericType containerItemType;
		CatGenericType assignableItemType;

		bool isReflectedArrayType;
		jitcat::Reflection::MemberFunctionInfo* arrayIndexFunction;
	};


} //End namespace jitcat::AST