/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ProductionToken.h"
#include "jitcat/ProductionTokenSet.h"

namespace jitcat::Tokenizer
{
	struct ParseToken;
	class TokenizerBase;
}


namespace jitcat::Grammar
{
	class ProductionTokenSet;


	class ProductionTerminalToken : public ProductionToken
	{
	public:
		ProductionTerminalToken(Tokenizer::TokenizerBase* tokenizer, unsigned short tokenId, unsigned short tokenSubType);
		virtual ~ProductionTerminalToken();
		bool matches(const Tokenizer::ParseToken* token) const;
		virtual ProductionTokenSet& getFirstSet() override final;
		virtual ProductionTokenSet& getFollowSet() override final;
		virtual bool getIsTerminal() const override final;
		virtual bool getIsEpsilon() const override final;
		virtual bool buildEpsilonContainment(std::vector<Production*>& productionStack) override final;
		virtual const char* getDescription() const override final;
		virtual const char* getSymbol() const override final;
		unsigned short getTokenId() const;
		unsigned short getTokenSubType() const;
		virtual ProductionTokenType getType() const override final; 
		virtual bool equals(const ProductionToken& other) const override final;
	private:
		unsigned short tokenId;
		unsigned short tokenSubType;
		Tokenizer::TokenizerBase* tokenizer;
		ProductionTokenSet firstSet;
		ProductionTokenSet followSet;
	};

} //End namespace jitcat::Grammar