/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Grammar
{
	class Production;
	class ProductionRule;
}

#include <string>

namespace jitcat::Parser
{

	//An item is a partial parse of a ProductionRule.
	//The tokenOffset defines the number of tokens that have been matched so far.
	class Item
	{
	public:
		Item():
			production(nullptr),
			rule(nullptr),
			tokenOffset(0)
		{}
		std::string toString() const;
		bool operator==(const Item& other) const;

	public:
		const Grammar::Production* production;
		const Grammar::ProductionRule* rule;
		unsigned int tokenOffset;
	};

} //End namespace jitcat::Parser