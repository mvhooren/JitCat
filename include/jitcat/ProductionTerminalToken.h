/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/ProductionToken.h"
namespace jitcat::Tokenizer
{
	class ParseToken;
	class TokenizerBase;
}


namespace jitcat::Grammar
{
	class ProductionTokenSet;


	class ProductionTerminalToken : public ProductionToken
	{
	public:
		ProductionTerminalToken(Tokenizer::TokenizerBase* tokenizer, int tokenId, int tokenSubType);
		virtual ~ProductionTerminalToken();
		bool matches(const Tokenizer::ParseToken* token) const;
		virtual ProductionTokenSet* getFirstSet() const;
		virtual ProductionTokenSet* getFollowSet() const;
		virtual bool getIsTerminal() const;
		virtual bool getIsEpsilon() const;
		virtual bool buildEpsilonContainment(std::vector<Production*>& productionStack);
		virtual const char* getDescription() const;
		virtual const char* getSymbol() const;
		int getTokenId() const;
		int getTokenSubType() const;
		virtual ProductionTokenType getType() const; 
		virtual bool equals(const ProductionToken& other) const;
	private:
		int tokenId;
		int tokenSubType;
		Tokenizer::TokenizerBase* tokenizer;
		ProductionTokenSet* firstSet;
		ProductionTokenSet* followSet;
	};

} //End namespace jitcat::Grammar