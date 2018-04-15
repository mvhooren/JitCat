/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "CommentToken.h"
#include "ConstantToken.h"
#include "ErrorToken.h"
#include "IdentifierToken.h"
#include "OneCharToken.h"
#include "TwoCharToken.h"
#include "WhitespaceToken.h"



namespace Cat
{
	int comment = CommentToken::getID();
	int ws		= WhitespaceToken::getID();
	int lit		= ConstantToken::getID();
	int id		= IdentifierToken::getID();
	int err		= ErrorToken::getID();
	int one		= OneCharToken::getID();
	int two		= TwoCharToken::getID();
};