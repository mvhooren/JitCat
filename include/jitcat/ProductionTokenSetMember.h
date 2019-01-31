/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <vector>
#include "jitcat/ProductionTokenType.h"

namespace jitcat::Grammar
{
	class ProductionTokenSet;
	class ProductionToken;

	class ProductionTokenSetMember
	{
	public:
		virtual ~ProductionTokenSetMember() {}
		virtual bool getIsSet() const = 0;
		virtual void addAllTerminals(std::vector<ProductionTokenSet*> recursionBlock, ProductionTokenSet* set) = 0;
		virtual bool getIsEpsilon() const = 0;
		virtual const char* getDescription() const = 0;
		virtual ProductionTokenType getType() const = 0; 
		virtual bool equals(const ProductionToken& other) const = 0;
		bool operator== (const ProductionToken& other) const;
	};

} //End namespace jitcat::Grammar