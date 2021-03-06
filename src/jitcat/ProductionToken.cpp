/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ProductionToken.h"
#include "jitcat/ProductionTokenSet.h"
#include <cassert>

using namespace jitcat::Grammar;

ProductionToken::ProductionToken() :
	containsEpsilon(TokenFlag::Unknown)
{

}


void ProductionToken::addAllTerminals(std::vector<ProductionTokenSet*> recursionBlock, ProductionTokenSet* set)
{
	set->addMemberIfNotPresent(this);
}


bool ProductionToken::getContainsEpsilon() 
{
	if (containsEpsilon == TokenFlag::Unknown)
	{
		assert(false);
		return false;
	}
	else
	{
		return containsEpsilon == TokenFlag::Yes;
	}
}


void ProductionToken::setContainsEpsilon(bool containsEpsilon_) 
{
	if (containsEpsilon_)
	{
		containsEpsilon = TokenFlag::Yes;
	}
	else
	{
		containsEpsilon = TokenFlag::No;
	}
}


bool ProductionToken::operator== (const ProductionToken& other) const
{
	if (other.getType() == getType())
	{
		return equals(other);
	}
	else
	{
		return false;
	}
}