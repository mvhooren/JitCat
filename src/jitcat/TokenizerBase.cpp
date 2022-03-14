/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/TokenizerBase.h"
#include "jitcat/Document.h"
#include "jitcat/Lexeme.h"
#include "jitcat/ParseToken.h"


using namespace jitcat::Tokenizer;


TokenizerBase::TokenizerBase()
{}


TokenizerBase::~TokenizerBase()
{
}


bool TokenizerBase::tokenize(Document& document)
{
	document.clearLineLookup();
	std::size_t currentPosition = 0;
	std::size_t size = document.getDocumentSize();
	while (currentPosition < size)
	{
		bool found = false;
		for (unsigned int i = 0; i < tokenFactories.size(); i++)
		{
			if (tokenFactories[i]->createIfMatch(document, currentPosition))
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			return false;
		}
	}
	document.addToken(ParseToken::getEofToken(document));
	document.addNewLine((int)currentPosition);
	return true;
}


void TokenizerBase::registerTokenFactory(std::unique_ptr<TokenFactory>&& factory)
{
	for (unsigned int i = 0; i < tokenFactories.size(); i++)
	{
		if (tokenFactories[i] == factory)
		{
			return;
		}
	}
	tokenFactories.emplace_back(std::move(factory));
}


const char* TokenizerBase::getTokenName(unsigned short tokenId, unsigned short tokenSubType) const
{
	if (tokenId == ParseToken::eofType)
		return "eof";
	for (unsigned int i = 0; i < tokenFactories.size(); i++)
	{
		if (tokenFactories[i]->getTokenID() == tokenId)
		{
			return tokenFactories[i]->getSubTypeName(tokenSubType);
		}
	}
	return "unknown";
}


const char* TokenizerBase::getTokenSymbol(unsigned short tokenId, unsigned short tokenSubType) const
{
	if (tokenId == ParseToken::eofType)
		return "eof";
	for (unsigned int i = 0; i < tokenFactories.size(); i++)
	{
		if (tokenFactories[i]->getTokenID() == tokenId)
		{
			return tokenFactories[i]->getSubTypeSymbol(tokenSubType);
		}
	}
	return "?";
}


bool TokenizerBase::isSuggestedToken(unsigned short tokenId, unsigned short tokenSubType) const
{
	for (unsigned int i = 0; i < tokenFactories.size(); i++)
	{
		if (tokenFactories[i]->getTokenID() == tokenId)
		{
			return tokenFactories[i]->isSuggestedToken(tokenSubType);
		}
	}
	return false;
}


unsigned short jitcat::Tokenizer::TokenizerBase::getNextFactoryID() const
{
	return (unsigned short)tokenFactories.size();
}
