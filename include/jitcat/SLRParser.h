/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class ExpressionErrorManager;
	class RuntimeContext;
}
namespace jitcat::AST
{
	class ASTNode;
}
namespace jitcat::Grammar
{
	class GrammarBase;
	class ProductionRule;
	class ProductionToken;
	class ProductionTokenSet;
}

#include "jitcat/Production.h"
#include "jitcat/SLRParseResult.h"
#include "jitcat/ParseToken.h"

#include <memory>
#include <set>
#include <string>
#include <vector>

namespace jitcat::Parser
{
	class DFAState;
	class Item;
	class StackItem;


	class SLRParser
	{
	public:
		SLRParser(const Grammar::GrammarBase* grammar): grammar(grammar) {}
	
		void createNFA();

	private:
		void buildNFA(DFAState* currentState, std::vector<DFAState*>& nfa);
		void convertNFAtoDFA(std::vector<DFAState*>& nfa);
		void epsilonClose(DFAState* state, DFAState* currentState, std::set<DFAState*>& recursionSet);
		DFAState* addState(DFAState* state, std::vector<DFAState*>& nfa);

		void cleanupAfterConversionToDFA(std::vector<DFAState*>& dfaToClean);

		void markReachableStates(DFAState* currentState);
	
		bool hasMultipleTransitionsWithToken(DFAState* state, Grammar::ProductionToken* token) const;
		void aggregateTokenTransitionIntoNewState(DFAState* state, Grammar::ProductionToken* token, std::vector<DFAState*>& nfa, const Item& item);

		//Reduces if possible and returns true if reduce move was done
		bool tryReduce(DFAState* currentState, std::vector<StackItem*>& stack, StackItem* nextToken, RuntimeContext* context) const;
		//Returns the new state if it is possible to shift the given token onto the stack.
		//Returns nullptr if the shift is not possible (error)
		DFAState* canShift(DFAState* currentState, StackItem* tokenToShift) const;
		bool isStackItemInFollowSet(StackItem* item, const Grammar::ProductionTokenSet& followSet) const;
		void printStack(const std::vector<StackItem*> stack) const;

		void scanForConflicts() const;

		std::string getShiftErrorMessage(DFAState* currentState, const std::string& errorTokenName) const;

	public:
		std::unique_ptr<SLRParseResult> parse(const std::vector<Tokenizer::ParseToken>& tokens, int whiteSpaceTokenID, int commentTokenID, RuntimeContext* context, ExpressionErrorManager* errorManager, void* errorSource) const;
		~SLRParser();

	private:
		const Grammar::GrammarBase* grammar;
		std::vector<DFAState*> dfa;
	};

} //End namespace jitcat::Parser