/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "StackItem.h"


class StackItemToken : public StackItem
{
public:
	StackItemToken(ParseToken* token):
		token(token) {}
		virtual const ParseToken* getTokenIfToken() const {return token;}
private:
	ParseToken* token;
};