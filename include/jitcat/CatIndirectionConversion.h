/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once

#include "jitcat/CatTypedExpression.h"

#include <memory>


namespace jitcat::AST
{
	class CatIndirectionConversion: public CatTypedExpression
	{
	public:
		CatIndirectionConversion(const Tokenizer::Lexeme& lexeme, const CatGenericType& outType, IndirectionConversionMode conversionMode, std::unique_ptr<CatTypedExpression> expressionToConvert);
		CatIndirectionConversion(const CatIndirectionConversion& other);

		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual bool isAssignable() const override final;
		virtual CatStatement* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual std::any execute(jitcat::CatRuntimeContext* runtimeContext) override final;
		virtual std::optional<bool> checkControlFlow(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext, bool& unreachableCodeDetected) override final;
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		IndirectionConversionMode getIndirectionConversionMode() const;
		const CatTypedExpression* getExpressionToConvert() const;

	private:
		std::unique_ptr<CatTypedExpression> expressionToConvert;
		IndirectionConversionMode conversionMode;

		CatGenericType convertedType;
		CatGenericType typeWithoutIndirection;
	};

}