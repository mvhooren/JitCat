/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatASTNode.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/TypeOwnershipSemantics.h"

#include <vector>
namespace jitcat::Reflection
{
	class TypeInfo;
}

namespace jitcat::AST
{
	class CatStaticScope;

	class CatTypeNode: public CatASTNode
	{
	public:
		CatTypeNode(const CatGenericType& type, const Tokenizer::Lexeme& lexeme);
		CatTypeNode(const std::string& name, Reflection::TypeOwnershipSemantics ownershipSemantics, const Tokenizer::Lexeme& lexeme);
		CatTypeNode(CatStaticScope* parentScope, const std::string& name, const Tokenizer::Lexeme& lexeme);
		CatTypeNode(std::unique_ptr<CatTypeNode> arrayItemType, Reflection::TypeOwnershipSemantics arrayOwnershipSemantics, const Tokenizer::Lexeme& lexeme);
		CatTypeNode(const CatTypeNode& other);

		virtual ~CatTypeNode();
		
		bool isKnownType() const;
		std::string getTypeName() const;
		const CatGenericType& getType() const;
		Reflection::TypeOwnershipSemantics getOwnershipSemantics() const;

		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		void setType(const CatGenericType& newType);

		bool defineCheck(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext, std::vector<const CatASTNode*>& loopDetectionStack);
		bool typeCheck(CatRuntimeContext* compileTimeContext, ExpressionErrorManager* errorManager, void* errorContext);

		void setOwnershipSemantics(Reflection::TypeOwnershipSemantics ownership);

	private:
		Reflection::TypeInfo* findType(CatRuntimeContext* compileTimeContext, const std::string* typeName);

	private:
		Reflection::TypeOwnershipSemantics ownershipSemantics;
		CatGenericType type;
		std::string name;
		bool knownType;
		bool isArrayType;
		std::unique_ptr<CatTypeNode> arrayItemType;
		std::unique_ptr<CatStaticScope> parentScope;
	};

}
