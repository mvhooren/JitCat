/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <vector>

class Document;
class ParseToken;

class Tokenizer
{
public:
	Tokenizer();
	virtual ~Tokenizer();
	bool tokenize(Document* document, std::vector<ParseToken*>& tokens, ParseToken* eofToken);
	void registerTokenFactory(ParseToken* factory);
	const char* getTokenName(int tokenId, int tokenSubType) const;
	const char* getTokenSymbol(int tokenId, int tokenSubType) const;
private:
	std::vector<ParseToken*> tokenFactories;
};