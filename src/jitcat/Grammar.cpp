/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "Grammar.h"
#include "CatLog.h"
#include "Production.h"
#include "ProductionEpsilonToken.h"
#include "ProductionNonTerminalToken.h"
#include "ProductionRule.h"
#include "ProductionTerminalToken.h"
#include "ProductionTokenSet.h"
#include "SLRParser.h"
#include "TokenFlag.h"
#include <iostream>



Grammar::Grammar(Tokenizer* tokenizer):
	tokenizer(tokenizer),
	rootProduction(nullptr),
	epsilonInstance(nullptr)
{}


Grammar::~Grammar()
{
	for (unsigned int i = 0; i < productions.size(); i++)
	{
		delete productions[i];
	}
	productions.clear();

	for (unsigned int i = 0; i < terminals.size(); i++)
	{
		delete terminals[i];
	}
	terminals.clear();

	delete epsilonInstance;
}


ProductionToken* Grammar::epsilon()
{
	if (epsilonInstance == nullptr)
	{
		epsilonInstance = new ProductionEpsilonToken();
	}
	return epsilonInstance;
}


SLRParser* Grammar::createSLRParser() const
{
	SLRParser* parser = new SLRParser();
	parser->createNFA(this);
	return parser;
}


void Grammar::rule(int productionId, std::initializer_list<ProductionToken*> tokens, SemanticAction action)
{
	ProductionRule* rule = new ProductionRule();
	for (auto iter : tokens)
	{
		rule->pushToken(iter);
	}
	rule->setSemanticAction(action);
	findOrCreateProduction(productionId)->addProductionRule(rule);
}


ProductionToken* Grammar::term(int tokenId, int tokenSubType)
{
	for (unsigned int i = 0; i < terminals.size(); i++)
	{
		if (terminals[i]->getTokenId() == tokenId &&
			terminals[i]->getTokenSubType()  == tokenSubType)
		{
			return terminals[i];
		}
	}
	ProductionTerminalToken * newTerminal = new ProductionTerminalToken(tokenizer, tokenId, tokenSubType);
	terminals.push_back(newTerminal);
	return newTerminal;
}


ProductionToken* Grammar::prod(int productionId)
{
	return new ProductionNonTerminalToken(findOrCreateProduction(productionId));
}


void Grammar::setRootProduction(int productionId, ProductionToken* eofToken)
{
	rootProduction = findOrCreateProduction(productionId);
	rootProduction->getFollowSet()->addMemberIfNotPresent(eofToken);
}


void Grammar::build()
{
	buildEpsilonContainment();
	buildFirstSets();
	buildFollowSets();
}


Production* Grammar::findOrCreateProduction(int productionId)
{
	Production* production;
	if (productions.find(productionId) != productions.end())
	{
		production = productions[productionId];
	}
	else
	{
		production = new Production(this, productionId);
		productions[productionId] = production;
	}
	return production;
}


void Grammar::buildEpsilonContainment()
{
	if (rootProduction != nullptr)
	{	
		std::map<int, Production*>::iterator iter;
		bool propagating = true;
		while (propagating)
		{
			propagating = false;
			for (iter = productions.begin(); iter != productions.end(); iter++)
			{
				std::vector<Production*> recursionStack;
				TokenFlag old = iter->second->getContainsEpsilon();
				iter->second->buildEpsilonContainment(recursionStack);
				if (iter->second->getContainsEpsilon() != old)
				{
					propagating = true;
				}
			}
		}
	}
#ifdef DEBUG_GRAMMAR
	CatLog::log("#######################################\n");
	CatLog::log("# Epsilon containment for productions #\n");
	CatLog::log("#######################################\n");
	std::map<int, Production*>::iterator iter;
	for (iter = productions.begin(); iter != productions.end(); iter++)
	{
		CatLog::log(getProductionName(iter->first));
		CatLog::log(": ");
		CatLog::log((int)iter->second->getContainsEpsilon());
		CatLog::log("\n");
	}
#endif
}


void Grammar::buildFirstSets()
{
	std::map<int, Production*>::iterator iter;
	for (iter = productions.begin(); iter != productions.end(); iter++)
	{
		iter->second->buildFirstSet();
	}
	for (iter = productions.begin(); iter != productions.end(); iter++)
	{
		iter->second->getFirstSet()->flatten();
	}
#ifdef DEBUG_GRAMMAR
	CatLog::log("##############\n");
	CatLog::log("# First sets #\n");
	CatLog::log("##############\n");
	for (iter = productions.begin(); iter != productions.end(); iter++)
	{
		CatLog::log(iter->second->getProductionName());
		CatLog::log(": ");
		ProductionTokenSet* set = iter->second->getFirstSet();
		
		for (unsigned int i = 0; i < set->getNumMembers(); i++)
		{
			if (i > 0)
			{
				CatLog::log(", ");
			}
			CatLog::log("'");
			CatLog::log(set->getMember(i)->getDescription());
			CatLog::log("'");
		}
		CatLog::log("\n");
	} 
#endif
}


void Grammar::buildFollowSets()
{
	std::map<int, Production*>::iterator iter;
	for (iter = productions.begin(); iter != productions.end(); iter++)
	{
		iter->second->buildFollowSets();
	}
	for (iter = productions.begin(); iter != productions.end(); iter++)
	{
		iter->second->getFollowSet()->flatten();
	}
	for (unsigned int i = 0; i < terminals.size(); i++)
	{
		terminals[i]->getFollowSet()->flatten();
	}
	#ifdef DEBUG_GRAMMAR
		CatLog::log("############################\n");
		CatLog::log("# Follow sets, productions #\n");
		CatLog::log("############################\n");
		for (iter = productions.begin(); iter != productions.end(); iter++)
		{
			CatLog::log(iter->second->getProductionName());
			CatLog::log(": ");
			ProductionTokenSet* set = iter->second->getFollowSet();

			for (unsigned int i = 0; i < set->getNumMembers(); i++)
			{
				if (i > 0)
				{
					CatLog::log(", ");
				}
				CatLog::log("'");
				CatLog::log(set->getMember(i)->getDescription());
				CatLog::log("'");
			}
			CatLog::log("\n");
		}
		CatLog::log("##########################\n");
		CatLog::log("# Follow sets, terminals #\n");
		CatLog::log("##########################\n");
		for (unsigned int i = 0; i < terminals.size(); i++)
		{
			ProductionTokenSet* set = terminals[i]->getFollowSet();
			CatLog::log(terminals[i]->getDescription());
			CatLog::log(": ");
			for (unsigned int j = 0; j < set->getNumMembers(); j++)
			{
				if (j > 0)
				{
					CatLog::log(", ");
				}
				CatLog::log("'");
				CatLog::log(set->getMember(j)->getDescription());
				CatLog::log("'");
			}
			CatLog::log("\n");
		}
	#endif
}