/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "DFAState.h"
#include "ProductionToken.h"

#include <sstream>


bool DFAState::addItem(const Item& item)
{
	for (unsigned int i = 0; i < items.size(); i++)
	{
		if (items[i] == item)
		{
			return false;
		}
	}
	items.push_back(item);
	return true;
}

void DFAState::addTransition(ProductionToken* token, DFAState* nextState, Item item)
{
	for (unsigned int i = 0; i < transitions.size(); i++)
	{
		if (transitions[i].nextState == nextState
			&& *transitions[i].transitionToken == *token)
		{
			return;
		}
	}
	DFAStateTransition transition;
	transition.nextState = nextState;
	transition.transitionToken = token;
	transition.item = item;
	transitions.push_back(transition);
}


std::string DFAState::toString() const
{
	std::stringstream stream;
	stream << "State: " << stateIndex << "\n";
	for (unsigned int i = 0; i < items.size(); i++)
	{
		stream << items[i].toString() << "\n";
	}
	for (unsigned int i = 0; i < transitions.size(); i++)
	{
		stream << transitions[i].transitionToken->getDescription() << " -> " << transitions[i].nextState->stateIndex << "\n";
	}
	return stream.str();
}


bool DFAState::operator==(const DFAState& other) const
{
	if (items.size() != other.items.size())
	{
		return false;
	}
	for (unsigned int i = 0; i < items.size(); i++)
	{
		bool itemIsInOtherList = false;
		for (unsigned int j = 0; j < other.items.size(); j++)
		{
			if (other.items[j] == items[i])
			{
				itemIsInOtherList = true;
				break;
			}
		}
		if (!itemIsInOtherList)
		{
			return false;
		}
	}
	return true;
}
