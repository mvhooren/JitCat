/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ProductionRule.h"
#include "Production.h"
#include "ProductionToken.h"
#include "ProductionTokenSet.h"
#include "Tools.h"

#include <stddef.h>

ProductionRule::ProductionRule()
{
	semanticAction = nullptr;
}


ProductionRule::~ProductionRule()
{
	for (int i = 0; i < (int)ruleTokens.size(); i++)
	{
		if (!ruleTokens[i]->getIsEpsilon()
			&& !ruleTokens[i]->getIsTerminal())
		{
			delete ruleTokens[i];
		}
	}
}


void ProductionRule::pushToken(ProductionToken* token)
{
	ruleTokens.push_back(token);
}


void ProductionRule::setSemanticAction(Grammar::SemanticAction action)
{
	semanticAction = action;
	if (action == nullptr)
	{
		//QQQ error, not allowed
	}
}


ASTNode* ProductionRule::executeSemanticAction(const ASTNodeParser& nodeParser) const
{
	if (semanticAction != nullptr)
	{
		return semanticAction(nodeParser);
	}
	return nullptr;
}


bool ProductionRule::buildEpsilonContainment(std::vector<Production*>& productionStack)
{
	bool allEpsilons = true;
	for (unsigned int i = 0; i < ruleTokens.size(); i++)
	{
		allEpsilons = allEpsilons && ruleTokens[i]->buildEpsilonContainment(productionStack);
	}
	return allEpsilons;
}


void ProductionRule::buildFirstSet(ProductionTokenSet* set)
{
	for (unsigned int i = 0; i < ruleTokens.size(); i++)
	{
		if (ruleTokens[i]->getIsEpsilon())
		{
			return;
		}
		if (ruleTokens[i]->getIsTerminal())
		{
			set->addMemberIfNotPresent(ruleTokens[i]);
			return;
		}
		else 
		{
			ProductionTokenSet* prodFirstSet = ruleTokens[i]->getFirstSet();
			if (prodFirstSet != set)
			{
				set->addMemberIfNotPresent(prodFirstSet);
			}
			if (!ruleTokens[i]->getContainsEpsilon())
			{
				return;
			}
		}
	}
}


void ProductionRule::buildFollowSets(Production* parentProduction)
{
	for (unsigned int i = 0; i < ruleTokens.size(); i++)
	{
		if (i < ruleTokens.size() - 1)
		{
			//Add following productions to follow of i until non-epsilon encountered
			bool allEpsilons = true;
			for (unsigned int j = i + 1; j < ruleTokens.size(); j++)
			{
				ruleTokens[i]->getFollowSet()->addMemberIfNotPresent(ruleTokens[j]->getFirstSet());
				if (!ruleTokens[j]->getContainsEpsilon())
				{
					allEpsilons = false;
					break;
				}
			}
			if (allEpsilons)
			{
				ruleTokens[i]->getFollowSet()->addMemberIfNotPresent(parentProduction->getFollowSet());
			}
		}
		else
		{
			ruleTokens[i]->getFollowSet()->addMemberIfNotPresent(parentProduction->getFollowSet());
		}
	}
}


std::size_t ProductionRule::getNumTokens() const
{
	return ruleTokens.size();
}


ProductionToken* ProductionRule::getToken(unsigned int index) const
{
	return ruleTokens[index];
}