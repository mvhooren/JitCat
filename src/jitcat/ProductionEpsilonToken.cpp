/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ProductionEpsilonToken.h"
#include "jitcat/ProductionTokenSet.h"

using namespace jitcat::Grammar;


ProductionEpsilonToken::ProductionEpsilonToken()
{
	firstSet = new ProductionTokenSet(false);
	firstSet->addMemberIfNotPresent(this);
	followSet = new ProductionTokenSet(true);
}


ProductionEpsilonToken::~ProductionEpsilonToken()
{
	delete firstSet;
	delete followSet;
}


bool ProductionEpsilonToken::getIsTerminal() const
{
	return true;
}


bool ProductionEpsilonToken::getIsEpsilon() const
{
	return true;
}


bool ProductionEpsilonToken::containsEpsilon() const
{
	return true;
}


bool ProductionEpsilonToken::buildEpsilonContainment(std::vector<Production*>& productionStack)
{
	setContainsEpsilon(true);
	return true;
}


ProductionTokenSet* ProductionEpsilonToken::getFirstSet() const
{
	return firstSet;
}


ProductionTokenSet* ProductionEpsilonToken::getFollowSet() const
{
	return followSet;
}


const char* ProductionEpsilonToken::getDescription() const
{
	return "Epsilon";
}


const char* ProductionEpsilonToken::getSymbol() const
{
	return "Eps";
}


ProductionTokenType ProductionEpsilonToken::getType() const
{
	return ProductionTokenType::Epsilon;
}


bool ProductionEpsilonToken::equals(const ProductionToken& other) const
{
	return other.getType() == ProductionTokenType::Epsilon;
}