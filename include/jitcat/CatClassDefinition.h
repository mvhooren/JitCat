/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatDefinition.h"
#include "jitcat/CatGenericType.h"


namespace jitcat::AST
{

	class CatClassDefinition: public CatDefinition
	{
	public:
		CatClassDefinition(const std::string& name, const Tokenizer::Lexeme& lexeme);
		virtual ~CatClassDefinition();

		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() override final;
		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) override final;

	private:
		std::string name;
	};

};
