/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ProductionTokenSet.h"
#include "Production.h"
#include "ProductionNonTerminalToken.h"
#include "ProductionTerminalToken.h"
#include "ParseToken.h"

#include <stddef.h>


ProductionTokenSet::ProductionTokenSet(bool disallowEpsilons):
	disallowEpsilons(disallowEpsilons)
{

}


void ProductionTokenSet::flatten()
{
	std::vector<ProductionTokenSet*> recursionBlock;
	addAllTerminals(recursionBlock, this);
	for (unsigned int i = 0; i < members.size(); i++)
	{
		if (members[i]->getIsSet())
		{
			members.erase(members.begin() + i);
			i--;
		}
	}
}


bool ProductionTokenSet::getIsSet() const
{
	return true;
}


bool ProductionTokenSet::containsEpsilon() const
{
	return false;
}


void ProductionTokenSet::addMemberIfNotPresent(ProductionTokenSetMember* member)
{
	if (disallowEpsilons
		&& member->getIsEpsilon())
	{
		return;
	}
	if (findMemberIndex(member) < 0)
	{
		members.push_back(member);
	}
}


const char* ProductionTokenSet::getDescription() const
{
	return "TokenSet";
}


std::size_t ProductionTokenSet::getNumMembers() const
{
	return members.size();
}


ProductionTokenSetMember* ProductionTokenSet::getMember(unsigned int index) const
{
	if (index < members.size())
	{
		return members[index];
	}
	return nullptr;
}


bool ProductionTokenSet::getIsEpsilon() const
{
	return false;
}


bool ProductionTokenSet::isInSet(const ParseToken* token) const
{
	ProductionTerminalToken terminal(nullptr, token->getTokenID(), token->getTokenSubType());
	for (int i = 0; i < (int)members.size(); i++)
	{
		if (*members[i] == terminal)
		{
			return true;
		}
	}
	return false;
}


bool ProductionTokenSet::isInSet(const Production* production) const
{
	for (int i = 0; i < (int)members.size(); i++)
	{
		if (members[i]->getType() == ProductionTokenType::NonTerminal
			&& static_cast<ProductionNonTerminalToken*>(members[i])->getProduction()->getProductionID() == production->getProductionID())
		{
			return true;
		}
	}
	return false;
}


bool ProductionTokenSet::isInSet(const ProductionToken* token) const
{
	for (int i = 0; i < (int)members.size(); i++)
	{
		if (*members[i] == *token)
		{
			return true;
		}
	}
	return false;
}


bool ProductionTokenSet::isInSet(const ProductionTokenSetMember* token) const
{
	for (int i = 0; i < (int)members.size(); i++)
	{
		if (token->getType() != ProductionTokenType::TokenSet
			&& members[i]->equals(*static_cast<const ProductionToken*>(token)))
		{
			return true;
		}
	}
	return false;
}


bool ProductionTokenSet::overlaps(const ProductionTokenSet* otherSet) const
{
	for (int i = 0; i < (int)members.size(); i++)
	{
		if (otherSet->isInSet(members[i]))
		{
			return true;
		}
	}
	return false;
}


ProductionTokenType ProductionTokenSet::getType() const
{
	return ProductionTokenType::TokenSet;
}


bool ProductionTokenSet::equals(const ProductionToken& other) const
{
	return false;
}


void ProductionTokenSet::addAllTerminals(std::vector<ProductionTokenSet*> recursionBlock, ProductionTokenSet* set)
{
	for (unsigned int i = 0; i < recursionBlock.size(); i++)
	{
		if (recursionBlock[i] == this)
		{
			return;
		}
	}
	recursionBlock.push_back(this);
	for (unsigned int i = 0; i < members.size(); i++)
	{
		members[i]->addAllTerminals(recursionBlock, set);
	}
}


int ProductionTokenSet::findMemberIndex(ProductionTokenSetMember* member)
{
	for (int i = 0; i < (int)members.size(); i++)
	{
		if (members[i] == member)
		{
			return i;
		}
	}
	return -1;
}