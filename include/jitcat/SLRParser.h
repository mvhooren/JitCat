/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class ASTNode;
class DFAState;
class Grammar;
class Item;
class ProductionRule;
class ProductionToken;
class ProductionTokenSet;
class RuntimeContext;
class StackItem;

#include "Production.h"
#include "SLRParseResult.h"
#include "ParseToken.h"

#include <set>
#include <string>
#include <vector>

class SLRParser
{
protected:
	friend class Grammar;
	friend class ASTNodeParser;

	SLRParser() {}
	
	void createNFA(const Grammar* grammar);

private:
	void buildNFA(DFAState* currentState, std::vector<DFAState*>& nfa);
	void convertNFAtoDFA(std::vector<DFAState*>& nfa);
	void epsilonClose(DFAState* state, DFAState* currentState, std::set<DFAState*>& recursionSet);
	DFAState* addState(DFAState* state, std::vector<DFAState*>& nfa);

	void cleanupAfterConversionToDFA(std::vector<DFAState*>& dfaToClean);

	void markReachableStates(DFAState* currentState);
	
	bool hasMultipleTransitionsWithToken(DFAState* state, ProductionToken* token) const;
	void aggregateTokenTransitionIntoNewState(DFAState* state, ProductionToken* token, std::vector<DFAState*>& nfa, const Item& item);

	//Reduces if possible and returns true if reduce move was done
	bool tryReduce(DFAState* currentState, std::vector<StackItem*>& stack, StackItem* nextToken, RuntimeContext* context) const;
	//Returns the new state if it is possible to shift the given token onto the stack.
	//Returns nullptr if the shift is not possible (error)
	DFAState* canShift(DFAState* currentState, StackItem* tokenToShift) const;
	bool isStackItemInFollowSet(StackItem* item, ProductionTokenSet* followSet) const;
	void printStack(const std::vector<StackItem*> stack) const;

	void scanForConflicts() const;

public:
	SLRParseResult* parse(const std::vector<ParseToken*>& tokens, int whiteSpaceTokenID, int commentTokenID, RuntimeContext* context) const;
	~SLRParser();

private:
	std::vector<DFAState*> dfa;
};