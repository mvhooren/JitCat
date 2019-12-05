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


bool TokenizerBase::tokenize(Document* document, std::vector<std::unique_ptr<ParseToken>>& tokens, ParseToken* eofToken)
{
	document->clearLineLookup();
	const char* data = document->getDocumentData().c_str();
	const char* position = data;
	std::size_t size = document->getDocumentSize();
	while (position < data + size)
	{
		bool found = false;
		for (unsigned int i = 0; i < tokenFactories.size(); i++)
		{
			ParseToken* token = tokenFactories[i]->createIfMatch(document, position);
			if (token != nullptr)
			{
				found = true;
				tokens.emplace_back(token);
				position += token->getLexeme().length();
				break;
			}
		}
		if (!found)
		{
			return false;
		}
	}
	tokens.emplace_back(eofToken);
	document->addNewLine((int)(position - data));
	return true;
}


void TokenizerBase::registerTokenFactory(std::unique_ptr<ParseToken> factory)
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


const char* TokenizerBase::getTokenName(int tokenId, int tokenSubType) const
{
	for (unsigned int i = 0; i < tokenFactories.size(); i++)
	{
		if (tokenFactories[i]->getTokenID() == tokenId)
		{
			return tokenFactories[i]->getSubTypeName(tokenSubType);
		}
	}
	return "unknown";
}


const char* TokenizerBase::getTokenSymbol(int tokenId, int tokenSubType) const
{
	for (unsigned int i = 0; i < tokenFactories.size(); i++)
	{
		if (tokenFactories[i]->getTokenID() == tokenId)
		{
			return tokenFactories[i]->getSubTypeSymbol(tokenSubType);
		}
	}
	return "?";
}


bool jitcat::Tokenizer::TokenizerBase::isSuggestedToken(int tokenId, int tokenSubType) const
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
