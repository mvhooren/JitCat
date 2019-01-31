/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class Document;
#include "Lexeme.h"

#include <memory>


class ParseToken
{
public:
	ParseToken();
	virtual ~ParseToken();
	virtual int getTokenID() const = 0;
	virtual const char* getTokenName() const = 0;
	virtual const char* getSubTypeName(int subType) const = 0;	
	virtual const char* getSubTypeSymbol(int subType) const = 0;
	virtual int getTokenSubType() const = 0;
	const Lexeme* getLexeme() const;
	virtual ParseToken* createIfMatch(Document* document, const char* currentPosition) const = 0;

protected:	
	static int getNextTokenID();

private:
	static int nextTokenID;

protected:
	std::unique_ptr<Lexeme> lexeme;
};