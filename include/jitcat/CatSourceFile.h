/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"
#include "jitcat/CatASTNodesDeclares.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/CatScope.h"

#include <vector>

namespace jitcat::Reflection
{
	class CustomTypeInfo;
	class CustomTypeInstance;
}
namespace jitcat::AST
{
	class CatDefinition;


	class CatSourceFile: public CatASTNode, public CatScope
	{
	public:
		CatSourceFile(const std::string& name, std::vector<std::unique_ptr<CatDefinition>>&& definitions, const Tokenizer::Lexeme& lexeme);
		CatSourceFile(const CatSourceFile& other);
		virtual ~CatSourceFile();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		const std::vector<CatClassDefinition*>& getClassDefinitions() const;
		const std::vector<CatFunctionDefinition*>& getFunctionDefinitions() const;

		bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext);

		// Inherited via CatScope
		virtual CatScopeID getScopeId() const override;
		virtual Reflection::CustomTypeInfo* getCustomType() override;

	private:
		void extractDefinitionLists();

	private:
		std::string name;

		//All definitions
		std::vector<std::unique_ptr<CatDefinition>> definitions;

		//All class definitions
		std::vector<CatClassDefinition*> classDefinitions;

		//All global function definitions
		std::vector<CatFunctionDefinition*> functionDefinitions;

		//All global variable declarations
		std::vector<CatVariableDefinition*> variableDefinitions;

		CatScopeID staticScopeId;
		std::unique_ptr<Reflection::CustomTypeInfo> scopeType;
		std::unique_ptr<Reflection::CustomTypeInstance> scopeInstance;
	};

};
