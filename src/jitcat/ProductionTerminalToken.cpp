/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ProductionTerminalToken.h"
#include "jitcat/ParseToken.h"
#include "jitcat/TokenizerBase.h"

#include <stddef.h>

using namespace jitcat::Grammar;
using namespace jitcat::Tokenizer;


ProductionTerminalToken::ProductionTerminalToken(TokenizerBase* tokenizer, int tokenId, int tokenSubType):
	tokenId(tokenId),
	tokenSubType(tokenSubType),
	tokenizer(tokenizer),
	firstSet(false),
	followSet(true)
{
	firstSet.addMemberIfNotPresent(this);
	setContainsEpsilon(false);
}


ProductionTerminalToken::~ProductionTerminalToken()
{
}


bool ProductionTerminalToken::matches(const ParseToken* token) const
{
	return token->getTokenID() == tokenId
		   && token->getTokenSubType() == tokenSubType;
}


ProductionTokenSet& ProductionTerminalToken::getFirstSet()
{
	return firstSet;
}


ProductionTokenSet& ProductionTerminalToken::getFollowSet()
{
	return followSet;
}


bool ProductionTerminalToken::getIsTerminal() const
{
	return true;
}


bool ProductionTerminalToken::getIsEpsilon() const
{
	return false;
}


bool ProductionTerminalToken::buildEpsilonContainment(std::vector<Production*>& productionStack)
{
	return false;
}


const char* ProductionTerminalToken::getDescription() const
{
	if (tokenizer != nullptr)
	{
		return tokenizer->getTokenName(tokenId, tokenSubType);
	}
	else
	{
		return "TOKENIZER_IS_nullptr";
	}
}


const char* ProductionTerminalToken::getSymbol() const
{
	if (tokenizer != nullptr)
	{
		return tokenizer->getTokenSymbol(tokenId, tokenSubType);
	}
	else
	{
		return "TOKENIZER_IS_nullptr";
	}
}

int ProductionTerminalToken::getTokenId() const
{
	return tokenId;
}


int ProductionTerminalToken::getTokenSubType() const
{
	return tokenSubType;
}


ProductionTokenType ProductionTerminalToken::getType() const
{
	return ProductionTokenType::Terminal;
}


bool ProductionTerminalToken::equals(const ProductionToken& other) const
{
	const ProductionTerminalToken* otherToken = static_cast<const ProductionTerminalToken*>(&other);
	return otherToken->tokenId == tokenId && otherToken->tokenSubType == tokenSubType;
}