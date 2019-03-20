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
	std::size_t tokenCount = tokenFactories.size();
	for (std::size_t i = 0; i < tokenCount; i++)
	{
		delete tokenFactories.back();
		tokenFactories.pop_back();
	}
}


bool TokenizerBase::tokenize(Document* document, std::vector<ParseToken*>& tokens, ParseToken* eofToken)
{
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
				tokens.push_back(token);
				position += token->getLexeme().length();
				break;
			}
		}
		if (!found)
		{
			return false;
		}
	}
	tokens.push_back(eofToken);
	return true;
}


void TokenizerBase::registerTokenFactory(ParseToken* factory)
{
	for (unsigned int i = 0; i < tokenFactories.size(); i++)
	{
		if (tokenFactories[i] == factory)
		{
			return;
		}
	}
	tokenFactories.push_back(factory);
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