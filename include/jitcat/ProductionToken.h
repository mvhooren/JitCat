/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ProductionTokenSetMember.h"
#include "jitcat/TokenFlag.h"
#include <vector>

namespace jitcat::Grammar
{
	class Production;
	class ProductionTokenSet;


	class ProductionToken : public ProductionTokenSetMember
	{
	public:
		ProductionToken();
		virtual ~ProductionToken() {};
		virtual const char* getSymbol() const = 0;
		virtual bool getIsTerminal() const = 0;
		virtual bool getIsEpsilon() const = 0;
		virtual ProductionTokenSet& getFirstSet() = 0;
		virtual ProductionTokenSet& getFollowSet() = 0;
		virtual bool getIsSet() const {return false;};
		virtual bool buildEpsilonContainment(std::vector<Production*>& productionStack) = 0;
		virtual void addAllTerminals(std::vector<ProductionTokenSet*> recursionBlock, ProductionTokenSet* set);
		virtual bool getContainsEpsilon();
		bool operator== (const ProductionToken& other) const;
	protected:
		void setContainsEpsilon(bool containsEpsilon_);
	private:
		TokenFlag containsEpsilon;
	};

} //End namespace jitcat::Grammar