/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatTypedExpression.h"

namespace jitcat::AST
{

	class CatLiteral: public CatTypedExpression
	{
	public:
		//CatLiteral(const Tokenizer::Lexeme& lexeme): CatTypedExpression(lexeme) {}
		CatLiteral(const std::any& value, CatGenericType type, const Tokenizer::Lexeme& lexeme): CatTypedExpression(lexeme), value(value), type(type) {}
		CatLiteral(const std::string& value, const Tokenizer::Lexeme& lexeme): CatTypedExpression(lexeme), value(value), type(CatGenericType::stringType) {}
		CatLiteral(float floatValue, const Tokenizer::Lexeme& lexeme): CatTypedExpression(lexeme), value(floatValue), type(CatGenericType::floatType) {}
		CatLiteral(int intValue, const Tokenizer::Lexeme& lexeme): CatTypedExpression(lexeme), value(intValue), type(CatGenericType::intType) {}
		CatLiteral(bool boolValue, const Tokenizer::Lexeme& lexeme): CatTypedExpression(lexeme), value(boolValue), type(CatGenericType::boolType) {}

		virtual CatGenericType getType() const override final {return type;} 
		virtual void print() const override final;
		virtual bool isConst() const override final {return true;}
		virtual CatTypedExpression* constCollapse(CatRuntimeContext* compileTimeContext) override final {return this;}
		virtual CatASTNodeType getNodeType() override final {return CatASTNodeType::Literal;}
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final {return value;};
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		const std::any& getValue() const;

	private:
		CatGenericType type;
		::std::any value;
	};

} // End namespace jitcat::AST