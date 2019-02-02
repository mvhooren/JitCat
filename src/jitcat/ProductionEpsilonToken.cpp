/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ProductionEpsilonToken.h"
#include "jitcat/ProductionTokenSet.h"

using namespace jitcat::Grammar;


ProductionEpsilonToken::ProductionEpsilonToken():
	firstSet(false),
	followSet(true)
{
	firstSet.addMemberIfNotPresent(this);
}


ProductionEpsilonToken::~ProductionEpsilonToken()
{
}


bool ProductionEpsilonToken::getIsTerminal() const
{
	return true;
}


bool ProductionEpsilonToken::getIsEpsilon() const
{
	return true;
}


bool ProductionEpsilonToken::buildEpsilonContainment(std::vector<Production*>& productionStack)
{
	setContainsEpsilon(true);
	return true;
}


ProductionTokenSet& ProductionEpsilonToken::getFirstSet()
{
	return firstSet;
}


ProductionTokenSet& ProductionEpsilonToken::getFollowSet()
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