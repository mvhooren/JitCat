/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ProductionTokenSetMember.h"
#include "jitcat/ParseToken.h"


#include <vector>

namespace jitcat::Grammar
{
	class Production;


	class ProductionTokenSet: public ProductionTokenSetMember
	{
	public:
		ProductionTokenSet(bool disallowEpsilons);
		virtual ~ProductionTokenSet();
		void flatten();
		virtual bool getIsSet() const override final;
		void addMemberIfNotPresent(ProductionTokenSetMember* member);
		virtual const char* getDescription() const override final;
		std::size_t getNumMembers() const;
		ProductionTokenSetMember* getMember(unsigned int index) const;
		virtual bool getIsEpsilon() const override final;
		bool isInSet(const Tokenizer::ParseToken* token) const;
		bool isInSet(const Production* production) const;
		bool isInSet(const ProductionToken* token) const;
		bool isInSet(const ProductionTokenSetMember* token) const;
		//Returns true if any token in otherSet is also in this set
		bool overlaps(const ProductionTokenSet& otherSet) const;

		virtual ProductionTokenType getType() const override final; 
		virtual bool equals(const ProductionToken& other) const override final;


	protected:
		virtual void addAllTerminals(std::vector<ProductionTokenSet*> recursionBlock, ProductionTokenSet* set) override final;
		int findMemberIndex(ProductionTokenSetMember* member);
		std::vector<ProductionTokenSetMember*> members;
		bool disallowEpsilons;
	};

} //End namespace jitcat::Grammar