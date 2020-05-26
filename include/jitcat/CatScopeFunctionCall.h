/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatTypedExpression.h"
#include "jitcat/FunctionSignature.h"

#include <memory>
#include <string>


namespace jitcat::AST
{
	class CatArgumentList;
    class CatMemberFunctionCall;
    class CatStaticFunctionCall;

    //This represents a function call that is not of the form "a.myMemberFunction()" or "MyType::myStaticFunction()", but just "myStaticOrMemberFunction()".
    //It could either be a static function call or a member function call on one of the scope objects of the current scope.
    //This is determined during type checking. During Const-collapse this node will be replaced by either CatMemberFunctionCall or CatStaticFunctionCall.
    class CatScopeFunctionCall: public CatTypedExpression, public Reflection::FunctionSignature
    {
    public:
        CatScopeFunctionCall(const std::string& name, CatArgumentList* arguments, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme);
        CatScopeFunctionCall(const CatScopeFunctionCall& other);
        virtual ~CatScopeFunctionCall();

		// Inherited via CatTypedExpression
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual std::any execute(CatRuntimeContext* runtimeContext) override final;
		virtual bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual const CatGenericType& getType() const override final;
		virtual bool isConst() const override final;
		virtual CatStatement* constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext) override final;

		// Inherited via FunctionSignature
		virtual const std::string& getLowerCaseFunctionName() const override final;
		virtual int getNumParameters() const override final;
		virtual const CatGenericType& getParameterType(int index) const override final;

    private:
        std::string name;
        std::string nameLowerCase;

        Tokenizer::Lexeme nameLexeme;
        Tokenizer::Lexeme lexeme;

        std::unique_ptr<CatArgumentList> arguments;
        std::unique_ptr<CatTypedExpression> functionCall;
    };
}