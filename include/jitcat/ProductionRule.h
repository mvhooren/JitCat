/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <vector>
#include "Grammar.h"
class ASTNode;
class ASTNodeParser;
class Production;
class ProductionToken;
class ProductionTokenSet;


class ProductionRule
{
public:
	ProductionRule();
	~ProductionRule();
	void pushToken(ProductionToken* token);
	void setSemanticAction(Grammar::SemanticAction action);
	ASTNode* executeSemanticAction(const ASTNodeParser& nodeParser) const;
	bool buildEpsilonContainment(std::vector<Production*>& productionStack);
	void buildFirstSet(ProductionTokenSet* set);
	void buildFollowSets(Production* parentProduction);
	std::size_t getNumTokens() const;
	ProductionToken* getToken(unsigned int index) const;
private:
	std::vector<ProductionToken*> ruleTokens;

	Grammar::SemanticAction semanticAction;
};