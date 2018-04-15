/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class ProductionRule;

#include "StackItem.h"

#include <vector>


class StackItemProduction : public StackItem
{
public:
	StackItemProduction(const Production* production, const ProductionRule* rule):
		production(production),
		rule(rule) {}
	virtual ~StackItemProduction();
	virtual const Production* getProductionIfProduction() const {return production;}
	void addChildItem(StackItem* stackItem)
	{
		children.push_back(stackItem);
	}
private:
	const Production* production;
	const ProductionRule* rule;
	std::vector<StackItem*> children;
};