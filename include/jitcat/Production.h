/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ProductionTokenSet.h"
#include "jitcat/TokenFlag.h"
#include <memory>
#include <vector>


namespace jitcat::Grammar
{
	class ProductionTokenSet;
	class ProductionRule;
	class GrammarBase;

	class Production
	{
	public:
		Production(GrammarBase* grammar, int productionId);
		~Production();
		void addProductionRule(std::unique_ptr<ProductionRule> rule);
		bool buildEpsilonContainment(std::vector<Production*>& productionStack);
		void buildFirstSet();
		void buildFollowSets();
		TokenFlag getContainsEpsilon();
		ProductionTokenSet& getFirstSet();
		ProductionTokenSet& getFollowSet();
		const ProductionTokenSet& getFirstSet() const;
		const ProductionTokenSet& getFollowSet() const;
		const char* getProductionName() const;
		std::size_t getNumRules() const;
		const ProductionRule& getRule(unsigned int index) const;
		int getProductionID() const;

	private:
		std::vector<std::unique_ptr<ProductionRule>> rules;
		int productionId;
		TokenFlag containsEpsilon;
		GrammarBase* grammar;
		ProductionTokenSet firstSet;
		ProductionTokenSet followSet;
	};

}