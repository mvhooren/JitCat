/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"

#include <memory>

namespace jitcat
{
	class CatRuntimeContext;
	class ExpressionErrorManager;
}

namespace jitcat::AST
{
	class CatTypedExpression;


	class CatRange: public CatASTNode
	{
	public:
		struct CatRangeIterator
		{
			int currentValue = 0;
		};

		CatRange(CatTypedExpression* rangeMax, const Tokenizer::Lexeme& lexeme);
		CatRange(CatTypedExpression* rangeMin, CatTypedExpression* rangeMax, const Tokenizer::Lexeme& lexeme);
		CatRange(CatTypedExpression* rangeMin, CatTypedExpression* rangeMax, CatTypedExpression* rangeStep, const Tokenizer::Lexeme& lexeme);
		CatRange(const CatRange& other);
		virtual ~CatRange();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		
		bool begin(CatRangeIterator& iterator, CatRuntimeContext* runtimeContext);
		bool next(CatRangeIterator& iterator, CatRuntimeContext* runtimeContext);

		bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext);
		bool hasAlwaysAtLeastOneIteration(CatRuntimeContext* compiletimeContext);
		
		CatASTNode* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext);

	private:
		bool isDefaultMin;
		std::unique_ptr<CatTypedExpression> rangeMin;
		std::unique_ptr<CatTypedExpression> rangeMax;
		bool isDefaultStep;
		std::unique_ptr<CatTypedExpression> rangeStep;
	};
}