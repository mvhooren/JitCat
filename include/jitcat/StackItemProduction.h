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

#include "jitcat/StackItem.h"

#include <vector>


namespace jitcat::Parser
{

	class StackItemProduction : public StackItem
	{
	public:
		StackItemProduction(const Grammar::Production* production, const Grammar::ProductionRule* rule):
			production(production),
			rule(rule) {}
		virtual ~StackItemProduction();
		virtual const Grammar::Production* getProductionIfProduction() const {return production;}
		const Grammar::ProductionRule* getProductionRule() const {return rule;}
		void addChildItem(StackItem* stackItem)
		{
			children.push_back(stackItem);
		}
	private:
		const Grammar::Production* production;
		const Grammar::ProductionRule* rule;
		std::vector<StackItem*> children;
	};

} //End namespace jitcat::Parser