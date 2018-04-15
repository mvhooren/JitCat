/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class ProductionToken;
#include "Item.h"
#include <vector>


//The state contains a set of items that define the possible moves from this state (shift or reduce)
class DFAState
{
public:
	struct DFAStateTransition
	{
		ProductionToken* transitionToken;
		DFAState* nextState;
		Item item;
	};

	bool addItem(const Item& item);
	void addTransition(ProductionToken* token, DFAState* nextState, Item item);
	std::string toString() const;
	bool operator==(const DFAState& other) const;

public:
	std::size_t stateIndex;
	std::vector<Item> items;
	std::vector<DFAStateTransition> transitions;
	bool reachable;
	bool isEpsilonClosed;
};