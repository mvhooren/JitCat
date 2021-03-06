/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTypedExpression.h"
#include "jitcat/Configuration.h"


#include <cassert>
#include <cmath>
#include <memory>
#include <string>
#include <sstream>

namespace jitcat::AST
{
	class CatInfixOperator: public CatTypedExpression
	{
	public:
		CatInfixOperator(CatTypedExpression* lhs, CatTypedExpression* rhs, CatInfixOperatorType operatorType, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& operatorLexeme);
		CatInfixOperator(const CatInfixOperator& other);

		virtual CatASTNode* copy() const override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual CatASTNodeType getNodeType() const override final {return CatASTNodeType::InfixOperator;}

		virtual CatStatement* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;

		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		virtual void print() const override final;

		CatTypedExpression* getLeft() const;
		CatTypedExpression* getRight() const;
		CatInfixOperatorType getOperatorType() const;

	private:
		inline std::any calculateExpression(CatRuntimeContext* runtimeContext);

		template<typename T, typename U, typename V>
		inline std::any calculateScalarExpression(const T& lValue, const U& rValue);
	
		template<typename T, typename U>
		inline std::any calculateStringExpression(const T& lValue, const U& rValue);

		inline std::any calculateStringExpression(const std::string& lValue, const std::string& rValue);

		inline std::any calculateBooleanExpression(bool lValue, bool rValue);

	private:
		std::unique_ptr<CatTypedExpression> overloadedOperator;
		
		Tokenizer::Lexeme operatorLexeme;

		std::unique_ptr<CatTypedExpression> lhs;
		CatInfixOperatorType oper;
		std::unique_ptr<CatTypedExpression> rhs;
		CatGenericType resultType;
	};

	#include "jitcat/CatInfixOperatorHeaderImplementation.h"

} //End namespace jitcat::AST

