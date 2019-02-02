/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Production.h"
#include "jitcat/ProductionEpsilonToken.h"
#include "jitcat/ProductionRule.h"
#include "jitcat/GrammarBase.h"

using namespace jitcat::Grammar;


Production::Production(GrammarBase* grammar, int productionId):
	grammar(grammar),
	productionId(productionId),
	containsEpsilon(TokenFlag::Unknown),
	firstSet(false),
	followSet(true)
{
}


Production::~Production()
{
}


void Production::addProductionRule(std::unique_ptr<ProductionRule> rule)
{
	rules.emplace_back(std::move(rule));
}


bool Production::buildEpsilonContainment(std::vector<Production*>& productionStack)
{
	if (containsEpsilon != TokenFlag::Unknown)
	{
		return containsEpsilon == TokenFlag::Yes;
	}
	for (unsigned int i = 0; i < productionStack.size(); i++)
	{
		if (productionStack[i] == this)
		{
			//recursion detected
			return false;
		}
	}
	productionStack.push_back(this);
	bool containsEpsilon_ = false;
	for (unsigned int i = 0; i < rules.size(); i++)
	{
		containsEpsilon_ = containsEpsilon_ || rules[i]->buildEpsilonContainment(productionStack);
	}
	containsEpsilon = containsEpsilon_ ? TokenFlag::Yes : TokenFlag::No;
	return containsEpsilon_;
}


void Production::buildFirstSet()
{
	for (unsigned int i = 0; i < rules.size(); i++)
	{
		rules[i]->buildFirstSet(firstSet);
	}
	if (containsEpsilon != TokenFlag::No)
	{
		firstSet.addMemberIfNotPresent(grammar->epsilon());
	}
}


void Production::buildFollowSets()
{
	for (unsigned int i = 0; i < rules.size(); i++)
	{
		rules[i]->buildFollowSets(this);
	}
}


TokenFlag Production::getContainsEpsilon()
{
	return containsEpsilon;
}


ProductionTokenSet& Production::getFirstSet()
{
	return firstSet;
}


ProductionTokenSet& Production::getFollowSet()
{
	return followSet;
}


const ProductionTokenSet& jitcat::Grammar::Production::getFirstSet() const
{
	return firstSet;
}


const ProductionTokenSet& jitcat::Grammar::Production::getFollowSet() const
{
	return followSet;
}


const char* Production::getProductionName() const
{
	return grammar->getProductionName(productionId);
}


std::size_t Production::getNumRules() const
{
	return rules.size();
}


const ProductionRule& Production::getRule(unsigned int index) const
{
	return *rules[index].get();
}


int Production::getProductionID() const
{
	return productionId;
}