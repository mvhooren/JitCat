/*
  This file is part of the JitCat library.

  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatDefinition.h"
#include "jitcat/CatGenericType.h"
#include "jitcat/ReflectableHandle.h"

#include <string>

namespace jitcat
{
	namespace Reflection
	{
		struct TypeMemberInfo;
	}
}

namespace jitcat::AST
{
	class CatTypeNode;
	
	class CatInheritanceDefinition : public CatDefinition
	{
	public:
		CatInheritanceDefinition(CatTypeNode* typeNode, const Tokenizer::Lexeme& nameLexeme, const Tokenizer::Lexeme& lexeme);
		CatInheritanceDefinition(const CatInheritanceDefinition& other);
		virtual ~CatInheritanceDefinition();

		// Inherited via CatASTNode
		virtual CatASTNode* copy() const override final;
		virtual void print() const override final;
		virtual CatASTNodeType getNodeType() const override final;

		virtual bool typeCheck(CatRuntimeContext* compileTimeContext) override final;
		bool postTypeCheck(CatRuntimeContext* compileTimeContext);

		CatGenericType getType() const;
		Reflection::TypeMemberInfo* getInheritedMember() const;

	private:
		const Tokenizer::Lexeme nameLexeme;
		std::unique_ptr<CatTypeNode> type;

		Reflection::TypeMemberInfo* inheritedMember;

		Reflection::ReflectableHandle errorManagerHandle;
	};
}