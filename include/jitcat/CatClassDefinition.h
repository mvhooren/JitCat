/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatDefinition.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatScope.h"

#include <functional>
#include <memory>
#include <vector>

namespace jitcat::Reflection
{
	class CustomTypeInfo;
}

namespace jitcat::AST
{
	class CatFunctionDefinition;
	class CatInheritanceDefinition;
	class CatVariableDefinition;
	
	class CatClassDefinition: public CatDefinition, public CatScope
	{
	public:
		CatClassDefinition(const std::string& name, std::vector<std::unique_ptr<CatDefinition>>&& definitions, const Tokenizer::Lexeme& lexeme, const Tokenizer::Lexeme& nameLexeme);
		CatClassDefinition(const CatClassDefinition& other);
		virtual ~CatClassDefinition();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) override final;

		bool isTriviallyCopyable() const;

		virtual Reflection::CustomTypeInfo* getCustomType() override final;
		virtual CatScopeID getScopeId() const override final;

		const std::string& getClassName() const;
		Tokenizer::Lexeme getClassNameLexeme() const;

		CatVariableDefinition* getVariableDefinitionByName(const std::string& name);
		CatFunctionDefinition* getFunctionDefinitionByName(const std::string& name);

		void enumerateMemberVariables(std::function<void(const CatGenericType&, const std::string&)>& enumerator) const;

		//Parses and injects code at the end of the function if it exists.
		//Injected code must be a single statement.
		bool injectCode(const std::string& functionName, const std::string& statement, CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext);

	private:
		bool generateConstructor(CatRuntimeContext* compileTimeContext);
		bool generateDestructor(CatRuntimeContext* compileTimeContext);

		void extractDefinitionLists();

	private:
		std::string name;
		Tokenizer::Lexeme nameLexeme;

		//All definitions
		std::vector<std::unique_ptr<CatDefinition>> definitions;

		//All class definitions
		std::vector<CatClassDefinition*> classDefinitions;

		//All global function definitions
		std::vector<CatFunctionDefinition*> functionDefinitions;

		//All variable definitions
		std::vector<CatVariableDefinition*> variableDefinitions;

		//All inheritance definitions
		std::vector<CatInheritanceDefinition*> inheritanceDefinitions;

		//Constructor/destructor
		std::unique_ptr<CatFunctionDefinition> generatedConstructor;
		std::unique_ptr<CatFunctionDefinition> generatedDestructor;

		CatScopeID scopeId;
		Reflection::CustomTypeInfo* customType;
	};

};
