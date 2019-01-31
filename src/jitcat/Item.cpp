/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Item.h"
#include "jitcat/Production.h"
#include "jitcat/ProductionRule.h"
#include "jitcat/ProductionToken.h"
#include "jitcat/ProductionTokenType.h"

#include <sstream>

using namespace jitcat::Grammar;
using namespace jitcat::Parser;

bool Item::operator==(const Item& other) const
{
	return other.production == production && other.rule == rule && other.tokenOffset == tokenOffset;
}


std::string Item::toString() const
{
	std::stringstream stream;
	for (unsigned int i = 0; i < rule->getNumTokens(); i++)
	{
		if (i == tokenOffset)
		{
			stream << ".";
		}
		if (rule->getToken(i)->getType() == Grammar::ProductionTokenType::NonTerminal)
		{
			stream << " " << rule->getToken(i)->getSymbol() << " ";
		}
		else
		{
			stream << rule->getToken(i)->getSymbol();
		}

	}
	if (tokenOffset >= rule->getNumTokens())
	{
		stream << ".";
	}

	stream << " -> " << production->getProductionName();
	return stream.str();
}