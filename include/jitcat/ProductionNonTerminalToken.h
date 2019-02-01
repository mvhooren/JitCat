/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ProductionToken.h"

namespace jitcat::Grammar
{
	class Production;


	class ProductionNonTerminalToken : public ProductionToken
	{
	public:
		ProductionNonTerminalToken(Production* production);
		virtual bool getIsTerminal() const;
		virtual bool getIsEpsilon() const;
		virtual bool buildEpsilonContainment(std::vector<Production*>& productionStack);
		virtual ProductionTokenSet* getFirstSet() const;
		virtual ProductionTokenSet* getFollowSet() const;
		virtual const char* getDescription() const;
		virtual const char* getSymbol() const;
		virtual bool getContainsEpsilon();
		virtual ProductionTokenType getType() const; 
		virtual bool equals(const ProductionToken& other) const;
		const Production* getProduction() const;
	private:
		Production* production;
	};

} //End namespace jitcat::Grammar