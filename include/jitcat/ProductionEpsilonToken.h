/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "ProductionToken.h"
class ProductionTokenSet;


class ProductionEpsilonToken: public ProductionToken
{
public:
	ProductionEpsilonToken();
	virtual ~ProductionEpsilonToken();
	virtual bool getIsTerminal() const;
	virtual bool getIsEpsilon() const;
	virtual bool containsEpsilon() const;
	virtual bool buildEpsilonContainment(std::vector<Production*>& productionStack);
	virtual ProductionTokenSet* getFirstSet() const;
	virtual ProductionTokenSet* getFollowSet() const;
	virtual const char* getDescription() const;
	virtual const char* getSymbol() const;
	virtual ProductionTokenType getType() const; 
	virtual bool equals(const ProductionToken& other) const;
private:
	ProductionTokenSet* firstSet;
	ProductionTokenSet* followSet;
};