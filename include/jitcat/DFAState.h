/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once
namespace jitcat::Grammar
{
	class ProductionToken;
}
#include "jitcat/Item.h"
#include <vector>

namespace jitcat::Parser
{

	//The state contains a set of items that define the possible moves from this state (shift or reduce)
	class DFAState
	{
	public:
		struct DFAStateTransition
		{
			Grammar::ProductionToken* transitionToken;
			DFAState* nextState;
			Item item;
		};

		bool addItem(const Item& item);
		void addTransition(Grammar::ProductionToken* token, DFAState* nextState, Item item);
		std::string toString() const;
		bool operator==(const DFAState& other) const;

	public:
		std::size_t stateIndex;
		std::vector<Item> items;
		std::vector<DFAStateTransition> transitions;
		bool reachable;
		bool isEpsilonClosed;
	};

} //End namespace jitcat::Parser