/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"
#include "jitcat/CatGenericType.h"

#include <any>
#include <memory>
#include <vector>


namespace jitcat::AST
{

	class CatTypedExpression;


	class CatArgumentList: public CatASTNode
	{
	public:
		CatArgumentList(const Tokenizer::Lexeme& lexeme, const std::vector<CatTypedExpression*>& argumentList);
		CatArgumentList(const CatArgumentList& other);

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext);
		void constCollapse(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext);
		bool getAllArgumentsAreConst() const;
		bool getArgumentIsConst(std::size_t argumentIndex) const;
		Tokenizer::Lexeme getArgumentLexeme(std::size_t argumentIndex) const;
		CatTypedExpression* releaseArgument(std::size_t argumentIndex);
		const CatTypedExpression* getArgument(std::size_t argumentIndex) const;

		void addArgument(std::unique_ptr<CatTypedExpression> argument);
		std::any executeArgument(std::size_t argumentIndex, CatRuntimeContext* context);
		void executeAllArguments(std::vector<std::any>& values, const std::vector<CatGenericType>& expectedTypes, CatRuntimeContext* context);

		const CatGenericType& getArgumentType(std::size_t argumentIndex) const;
		std::size_t getNumArguments() const;
		const std::vector<CatGenericType>& getArgumentTypes() const;

		bool applyIndirectionConversions(const std::vector<CatGenericType>& expectedTypes, const std::string& functionName, CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext);

	private:
		std::vector<std::unique_ptr<CatTypedExpression>> arguments;
		std::vector<CatGenericType> argumentTypes;
	};


} //End namespace jitcat::AST