/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CatASTNode.h"
#include "CatASTNodesDeclares.h"
#include "CatGenericType.h"

#include <vector>


namespace jitcat::AST
{
	class CatDefinition;


	class CatSourceFile: public CatASTNode
	{
	public:
		CatSourceFile(const std::string& name, std::vector<std::unique_ptr<CatDefinition>>&& definitions, const Tokenizer::Lexeme& lexeme);
		virtual ~CatSourceFile();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;

		const std::vector<const CatClassDefinition*>& getClassDefinitions() const;
		const std::vector<const CatFunctionDefinition*>& getFunctionDefinitions() const;

		bool typeCheck(CatRuntimeContext* compiletimeContext, ExpressionErrorManager* errorManager, void* errorContext);

	private:
		std::string name;

		//All definitions
		std::vector<std::unique_ptr<CatDefinition>> definitions;

		//All class definitions
		std::vector<const CatClassDefinition*> classDefinitions;

		//All function definitions
		std::vector<const CatFunctionDefinition*> functionDefinitions;
	};

};
