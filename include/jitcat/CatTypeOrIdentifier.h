/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"
#include "jitcat/TypeOwnershipSemantics.h"

namespace jitcat::AST
{
	class CatArgumentList;
	class CatErrorExpression;
	class CatFunctionOrConstructor;
	class CatIdentifier;
	class CatMemberFunctionCall;
	class CatOperatorNew;
	class CatStaticFunctionCall;
	class CatStaticIdentifier;
	class CatStaticScope;
	class CatTypedExpression;
	class CatTypeNode;

	//Represents and identifier, optionally preceded by a static scope and optionally decorated with ownership semantics.
	//This is a temporary AST node that will be disambiguated during the parse to various other AST nodes such as identifiers, function calls and type names.
	class CatTypeOrIdentifier: public CatASTNode
	{
		enum class TypeOrIdentifier
		{
			Ambiguous,
			Type
		};
	public:
		CatTypeOrIdentifier(CatTypeNode* type, Reflection::TypeOwnershipSemantics ownershipSemantics, const Tokenizer::Lexeme& lexeme);
		CatTypeOrIdentifier(const std::string& identifier, const Tokenizer::Lexeme& identifierLexeme, Reflection::TypeOwnershipSemantics ownershipSemantics, const Tokenizer::Lexeme& lexeme);
		CatTypeOrIdentifier(CatStaticScope* parentScope, const std::string& identifier, const Tokenizer::Lexeme& identifierLexeme, Reflection::TypeOwnershipSemantics ownershipSemantics, const Tokenizer::Lexeme& lexeme);
		CatTypeOrIdentifier(const CatTypeOrIdentifier& other);
		virtual ~CatTypeOrIdentifier();

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		bool isType() const;
		bool isAmbiguous() const;
		bool hasParentScope() const;

		//Calling one of these functions will potentially release the parentNode or typeNode unique pointers
		//After calling these, this class should be destructed.
		CatIdentifier* toIdentifier();
		CatStaticIdentifier* toStaticIdentifier();
		CatTypeNode* toType();
		CatStaticScope* toStaticScope();
		CatErrorExpression* toErrorExpression(const std::string& errorMessage);
		CatASTNode* toFunctionCall(CatArgumentList* argumentList);
		CatOperatorNew* toConstructorCall(CatArgumentList* argumentList);
		//This is an exception to the others, the CatTypeOrIdentifier is preserved in CatFunctionOrConstructor and should not be deleted. 
		CatFunctionOrConstructor* toFunctionOrConstructorCall(CatArgumentList* argumentList);

	private:
		TypeOrIdentifier typeOrIdentifier;

		std::unique_ptr<CatStaticScope> parentScope;
		std::unique_ptr<CatTypeNode> typeNode;

		const std::string identifier;
		const Tokenizer::Lexeme identifierLexeme;
		Reflection::TypeOwnershipSemantics ownershipSemantics;
	};
}
