/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/GrammarBase.h"
#include "jitcat/CatLog.h"
#include "jitcat/Configuration.h"
#include "jitcat/Production.h"
#include "jitcat/ProductionEpsilonToken.h"
#include "jitcat/ProductionNonTerminalToken.h"
#include "jitcat/ProductionRule.h"
#include "jitcat/ProductionTerminalToken.h"
#include "jitcat/ProductionTokenSet.h"
#include "jitcat/SLRParser.h"
#include "jitcat/TokenFlag.h"
#include <iostream>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Grammar;
using namespace jitcat::Parser;
using namespace jitcat::Tokenizer;
using namespace jitcat::Tools;


GrammarBase::GrammarBase(TokenizerBase* tokenizer):
	tokenizer(tokenizer),
	rootProduction(nullptr),
	epsilonInstance(nullptr)
{}


GrammarBase::~GrammarBase()
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


ProductionToken* GrammarBase::epsilon()
{
	if (epsilonInstance == nullptr)
	{
		epsilonInstance = new ProductionEpsilonToken();
	}
	return epsilonInstance;
}


std::unique_ptr<Parser::SLRParser> GrammarBase::createSLRParser() const
{
	std::unique_ptr<SLRParser> parser(new SLRParser(this));
	parser->createNFA();
	return parser;
}


const Tokenizer::TokenizerBase* jitcat::Grammar::GrammarBase::getTokenizer() const
{
	return tokenizer;
}


void GrammarBase::rule(int productionId, std::initializer_list<ProductionToken*> tokens, SemanticAction action)
{
	std::unique_ptr<ProductionRule> rule(std::make_unique<ProductionRule>());
	for (auto iter : tokens)
	{
		rule->pushToken(iter);
	}
	rule->setSemanticAction(action);
	findOrCreateProduction(productionId)->addProductionRule(std::move(rule));
}


ProductionToken* GrammarBase::term(int tokenId, int tokenSubType)
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


ProductionToken* GrammarBase::prod(int productionId)
{
	return new ProductionNonTerminalToken(findOrCreateProduction(productionId));
}


void GrammarBase::setRootProduction(int productionId, ProductionToken* eofToken)
{
	rootProduction = findOrCreateProduction(productionId);
	rootProduction->getFollowSet().addMemberIfNotPresent(eofToken);
}


void GrammarBase::build()
{
	buildEpsilonContainment();
	buildFirstSets();
	buildFollowSets();
}


Production* GrammarBase::findOrCreateProduction(int productionId)
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


void GrammarBase::buildEpsilonContainment()
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
	if constexpr (Configuration::debugGrammar)
	{
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
	}
}


void GrammarBase::buildFirstSets()
{
	std::map<int, Production*>::iterator iter;
	for (iter = productions.begin(); iter != productions.end(); iter++)
	{
		iter->second->buildFirstSet();
	}
	for (iter = productions.begin(); iter != productions.end(); iter++)
	{
		iter->second->getFirstSet().flatten();
	}
	if constexpr (Configuration::debugGrammar)
	{
		CatLog::log("##############\n");
		CatLog::log("# First sets #\n");
		CatLog::log("##############\n");
		for (iter = productions.begin(); iter != productions.end(); iter++)
		{
			CatLog::log(iter->second->getProductionName());
			CatLog::log(": ");
			ProductionTokenSet& set = iter->second->getFirstSet();

			for (unsigned int i = 0; i < set.getNumMembers(); i++)
			{
				if (i > 0)
				{
					CatLog::log(", ");
				}
				CatLog::log("'");
				CatLog::log(set.getMember(i)->getDescription());
				CatLog::log("'");
			}
			CatLog::log("\n");
		}
	}
}


void GrammarBase::buildFollowSets()
{
	std::map<int, Production*>::iterator iter;
	for (iter = productions.begin(); iter != productions.end(); iter++)
	{
		iter->second->buildFollowSets();
	}
	for (iter = productions.begin(); iter != productions.end(); iter++)
	{
		iter->second->getFollowSet().flatten();
	}
	for (unsigned int i = 0; i < terminals.size(); i++)
	{
		terminals[i]->getFollowSet().flatten();
	}
	if constexpr (Configuration::debugGrammar)
	{
		CatLog::log("############################\n");
		CatLog::log("# Follow sets, productions #\n");
		CatLog::log("############################\n");
		for (iter = productions.begin(); iter != productions.end(); iter++)
		{
			CatLog::log(iter->second->getProductionName());
			CatLog::log(": ");
			ProductionTokenSet& set = iter->second->getFollowSet();

			for (unsigned int i = 0; i < set.getNumMembers(); i++)
			{
				if (i > 0)
				{
					CatLog::log(", ");
				}
				CatLog::log("'");
				CatLog::log(set.getMember(i)->getDescription());
				CatLog::log("'");
			}
			CatLog::log("\n");
		}
		CatLog::log("##########################\n");
		CatLog::log("# Follow sets, terminals #\n");
		CatLog::log("##########################\n");
		for (unsigned int i = 0; i < terminals.size(); i++)
		{
			ProductionTokenSet& set = terminals[i]->getFollowSet();
			CatLog::log(terminals[i]->getDescription());
			CatLog::log(": ");
			for (unsigned int j = 0; j < set.getNumMembers(); j++)
			{
				if (j > 0)
				{
					CatLog::log(", ");
				}
				CatLog::log("'");
				CatLog::log(set.getMember(j)->getDescription());
				CatLog::log("'");
			}
			CatLog::log("\n");
		}
	}
}