/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ProductionToken.h"
#include "jitcat/ProductionTokenSet.h"


namespace jitcat::Grammar
{
	class ProductionTokenSet;


	class ProductionEpsilonToken: public ProductionToken
	{
	public:
		ProductionEpsilonToken();
		virtual ~ProductionEpsilonToken();
		virtual bool getIsTerminal() const override final;
		virtual bool getIsEpsilon() const override final;
		virtual bool buildEpsilonContainment(std::vector<Production*>& productionStack) override final;
		virtual ProductionTokenSet& getFirstSet() override final;
		virtual ProductionTokenSet& getFollowSet() override final;
		virtual const char* getDescription() const override final;
		virtual const char* getSymbol() const override final;
		virtual ProductionTokenType getType() const override final;  
		virtual bool equals(const ProductionToken& other) const override final;
	private:
		ProductionTokenSet firstSet;
		ProductionTokenSet followSet;
	};

} //End namespace jitcat::Grammar