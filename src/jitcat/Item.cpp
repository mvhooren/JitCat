/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "Item.h"
#include "Production.h"
#include "ProductionRule.h"
#include "ProductionToken.h"
#include "ProductionTokenType.h"

#include <sstream>


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
		if (rule->getToken(i)->getType() == ProductionTokenType::NonTerminal)
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