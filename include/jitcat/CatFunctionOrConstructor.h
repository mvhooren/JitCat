/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once


#include "jitcat/CatASTNode.h"

#include <memory>


namespace jitcat::AST
{
	class CatArgumentList;
	class CatTypeOrIdentifier;

	//Represents a function call that can either be a constructor call used in operator new or a normal function call.
	//This is a temporary AST node that will be disambiguated during the parse to various other AST nodes.
	class CatFunctionOrConstructor: public CatASTNode
	{
	public:
		CatFunctionOrConstructor(CatTypeOrIdentifier* typeOrIdentifier, CatArgumentList* argumentList, const Tokenizer::Lexeme& lexeme);
		CatFunctionOrConstructor(const CatFunctionOrConstructor& other);
		virtual ~CatFunctionOrConstructor();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		//These functions disambiguate this class to either a function call or a constructor call.
		//After one of these functions has been called, typeOrIdentifier and arguments will be 
		//released and this class should be destroyed.
		CatASTNode* toConstructorCall();
		CatASTNode* toFunctionCall();

	private:
		std::unique_ptr<CatTypeOrIdentifier> typeOrIdentifier;
		std::unique_ptr<CatArgumentList> argumentList;
	};
}