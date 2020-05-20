/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatTypedExpression.h"
#include "jitcat/Configuration.h"


namespace jitcat::AST
{

	class CatLiteral: public CatTypedExpression
	{
	public:
		CatLiteral(const std::any& value, CatGenericType type, const Tokenizer::Lexeme& lexeme);
		CatLiteral(const Configuration::CatString& value, const Tokenizer::Lexeme& lexeme);
		CatLiteral(float floatValue, const Tokenizer::Lexeme& lexeme);
		CatLiteral(double doubleValue, const Tokenizer::Lexeme& lexeme);
		CatLiteral(int intValue, const Tokenizer::Lexeme& lexeme);
		CatLiteral(bool boolValue, const Tokenizer::Lexeme& lexeme);
		CatLiteral(const CatLiteral& other);

		virtual CatASTNode* copy() const override final;
		virtual const CatGenericType& getType() const override final {return type;}
		virtual void print() const override final;
		virtual bool isConst() const override final {return true;}
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final {return this;}
		virtual CatASTNodeType getNodeType() const override final {return CatASTNodeType::Literal;}
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final {return value;};
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		const std::any& getValue() const;

	private:
		CatGenericType type;
		::std::any value;
	};

} // End namespace jitcat::AST