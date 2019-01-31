/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ProductionNonTerminalToken.h"
#include "jitcat/Production.h"

using namespace jitcat::Grammar;

ProductionNonTerminalToken::ProductionNonTerminalToken(Production* production):
	production(production)
{

}


bool ProductionNonTerminalToken::getIsTerminal() const
{
	return false;
}


bool ProductionNonTerminalToken::getIsEpsilon() const
{
	return false;
}


bool ProductionNonTerminalToken::buildEpsilonContainment(std::vector<Production*>& productionStack)
{
	return production->buildEpsilonContainment(productionStack);
}


ProductionTokenSet* ProductionNonTerminalToken::getFirstSet() const
{
	return production->getFirstSet();
}


ProductionTokenSet* ProductionNonTerminalToken::getFollowSet() const
{
	return production->getFollowSet();
}


const char* ProductionNonTerminalToken::getDescription() const
{
	return production->getProductionName();
}


const char* ProductionNonTerminalToken::getSymbol() const
{
	return production->getProductionName();
}


bool ProductionNonTerminalToken::getContainsEpsilon()
{
	return production->getContainsEpsilon() == TokenFlag::Yes;
}


ProductionTokenType ProductionNonTerminalToken::getType() const
{
	return ProductionTokenType::NonTerminal;
}


bool ProductionNonTerminalToken::equals(const ProductionToken& other) const
{
	const ProductionNonTerminalToken* otherToken = static_cast<const ProductionNonTerminalToken*>(&other);
	return otherToken->production == production;
}


const Production* ProductionNonTerminalToken::getProduction() const
{
	return production;
}