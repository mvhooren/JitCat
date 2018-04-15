/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class ASTNode;
class DFAState;
class ParseToken;
class Production;

#include <stddef.h>

class StackItem
{
public:
	StackItem():
		state(nullptr),
		astNode(nullptr)
	{}
	virtual ~StackItem() {}
	DFAState* state;
	ASTNode* astNode;
	virtual const ParseToken* getTokenIfToken() const {return 0;}
	virtual const Production* getProductionIfProduction() const {return 0;}
};