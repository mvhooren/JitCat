/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <vector>
#include "jitcat/GrammarBase.h"

namespace jitcat
{
	namespace AST
	{
		class ASTNode;
	}
	namespace Parser
	{
		class ASTNodeParser;
	}
	namespace Grammar
	{
		class Production;
		class ProductionToken;
		class ProductionTokenSet;

		class ProductionRule
		{
		public:
			ProductionRule();
			~ProductionRule();
			void pushToken(ProductionToken* token);
			void setSemanticAction(GrammarBase::SemanticAction action);
			AST::ASTNode* executeSemanticAction(const Parser::ASTNodeParser& nodeParser) const;
			bool buildEpsilonContainment(std::vector<Production*>& productionStack);
			void buildFirstSet(ProductionTokenSet& set);
			void buildFollowSets(Production* parentProduction);
			std::size_t getNumTokens() const;
			ProductionToken* getToken(unsigned int index) const;
		private:
			std::vector<ProductionToken*> ruleTokens;

			GrammarBase::SemanticAction semanticAction;
		};

	} //End namespace Grammar
} //End namespace jitcat