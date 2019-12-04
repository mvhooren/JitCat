/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once
namespace jitcat
{
	class CatRuntimeContext;
	namespace Reflection
	{
		class CustomTypeInfo;
	}
}
#include "jitcat/CatASTNode.h"
#include "jitcat/TypeInfoDeleter.h"

#include <memory>
#include <string>
#include <vector>


namespace jitcat::AST
{
	class CatVariableDeclaration;
	class CatTypeNode;

	class CatFunctionParameterDefinitions: public CatASTNode
	{
	public:
		CatFunctionParameterDefinitions(const std::vector<CatVariableDeclaration*>& parameterDeclarations, const Tokenizer::Lexeme& lexeme);
		CatFunctionParameterDefinitions(const CatFunctionParameterDefinitions& other);
		virtual ~CatFunctionParameterDefinitions();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		bool typeCheck(CatRuntimeContext* runtimeContext, ExpressionErrorManager* errorManager, void* errorContext);
		Reflection::CustomTypeInfo* getCustomType() const;
		int getNumParameters() const;
		const std::string& getParameterName(int index) const;
		const CatTypeNode* getParameterType(int index) const;
		const Tokenizer::Lexeme& getParameterLexeme(int index) const;

	private:
		std::vector<std::unique_ptr<CatVariableDeclaration>> parameters;
		std::unique_ptr<Reflection::CustomTypeInfo, Reflection::TypeInfoDeleter> customType;
	};

}
