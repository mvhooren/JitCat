/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/SLRParser.h"
#include "jitcat/ASTNode.h"
#include "jitcat/ASTNodeParser.h"
#include "jitcat/CatLog.h"
#include "jitcat/DFAState.h"
#include "jitcat/GrammarBase.h"
#include "jitcat/Lexeme.h"
#include "jitcat/Production.h"
#include "jitcat/ProductionEpsilonToken.h"
#include "jitcat/ProductionRule.h"
#include "jitcat/ProductionNonTerminalToken.h"
#include "jitcat/ProductionTerminalToken.h"
#include "jitcat/ProductionToken.h"
#include "jitcat/ProductionTokenSet.h"
#include "jitcat/SLRParseResult.h"
#include "jitcat/StackItemProduction.h"
#include "jitcat/StackItemToken.h"
#include "jitcat/Tools.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Grammar;
using namespace jitcat::Parser;
using namespace jitcat::Tokenizer;
using namespace jitcat::Tools;


void SLRParser::createNFA(const GrammarBase* grammar)
{
	std::vector<DFAState*>& nfa = dfa;
	Production* prod = grammar->rootProduction;
	if (prod->getNumRules() == 1
		&& prod->getRule(0)->getNumTokens() == 1
		&& !prod->getRule(0)->getToken(0)->getIsTerminal())
	{
		//The root production should contain only a single production
		DFAState* rootState = new DFAState();
		rootState->isEpsilonClosed = false;
		rootState->stateIndex = 0;
		rootState->reachable = false;
		Item rootItem;
		rootItem.production = prod;
		rootItem.rule = prod->getRule(0);
		rootItem.tokenOffset = 0;
		rootState->addItem(rootItem);
		nfa.push_back(rootState);
		buildNFA(rootState, nfa);
		
		#ifdef DEBUG_GRAMMAR
			CatLog::log("\n\n###### nfa #######\n\n");
			for (unsigned int i = 0; i < nfa.size(); i++)
			{
				CatLog::log(nfa[i]->toString().c_str());
				CatLog::log("\n");
			}
		#endif
		convertNFAtoDFA(nfa);
		#ifdef DEBUG_GRAMMAR
			CatLog::log("\n\n###### Final dfa #######\n\n");
			for (unsigned int i = 0; i < nfa.size(); i++)
			{
				CatLog::log(nfa[i]->toString().c_str());
				CatLog::log("\n");
			}
			scanForConflicts();
		#endif
	}
	else
	{
		CatLog::log("ERROR: The root production of the grammar should contain just a single rule and that rule should contain just a single non-terminal token.\n");
	}
}


void SLRParser::buildNFA(DFAState* currentState, std::vector<DFAState*>& nfa)
{
	for (unsigned int i = 0; i < currentState->items.size(); i++)
	{
		const Item& currentItem = currentState->items[i];
		if (currentItem.tokenOffset < currentItem.rule->getNumTokens()
			&& !(currentItem.rule->getNumTokens() == 1 && currentItem.rule->getToken(0)->getType() == ProductionTokenType::Epsilon))
		{
			//State transition
			Item nextItem(currentItem);
			nextItem.tokenOffset++;
			DFAState* nextState = new DFAState();
			nextState->addItem(nextItem);
			DFAState* newState = addState(nextState, nfa);
			ProductionToken* currentItemNextToken = currentItem.rule->getToken(currentItem.tokenOffset);
			currentState->addTransition(currentItemNextToken, newState, currentState->items[i]);
			if (newState == nextState)
			{
				//this state did not yet exist
				buildNFA(newState, nfa);
			}
			
			if (currentItemNextToken->getType() == ProductionTokenType::NonTerminal)
			{
				const Production* production = static_cast<const ProductionNonTerminalToken*>(currentItemNextToken)->getProduction();
				for (unsigned int j = 0; j < production->getNumRules(); j++)
				{
					const ProductionRule* rule = production->getRule(j);
					nextItem.production = production;
					nextItem.rule = rule;
					nextItem.tokenOffset = 0;
					nextState = new DFAState();
					nextState->addItem(nextItem);
					newState = addState(nextState, nfa);
					static ProductionEpsilonToken epsilonToken;
					Item epsilonItem;
					epsilonItem.production = nullptr;
					epsilonItem.rule = nullptr;
					epsilonItem.tokenOffset = 0;
					currentState->addTransition(&epsilonToken, newState, epsilonItem);
					if (newState == nextState)
					{
						//this state did not yet exist
						buildNFA(newState, nfa);
					}
				}
			}
		}
	}
}


void SLRParser::convertNFAtoDFA(std::vector<DFAState*>& nfa)
{
	DFAState* startingState = nfa[0];

	std::set<DFAState*> recursionSet;
	epsilonClose(startingState, startingState, recursionSet);

	cleanupAfterConversionToDFA(nfa);

	markReachableStates(startingState);
	for (int i = 0; i < (int)nfa.size(); i++)
	{
		if (!nfa[i]->reachable)
		{
			delete nfa[i];
			nfa.erase(nfa.begin() + i);
			i--;
		}
		else
		{
			nfa[i]->stateIndex = i;
		}
	}
}


void SLRParser::epsilonClose(DFAState* state, DFAState* currentState, std::set<DFAState*>& recursionSet)
{
	if (state != currentState)
	{
		for (unsigned int j = 0; j < currentState->items.size(); j++)
		{
			state->addItem(currentState->items[j]);
		}
	}
	else
	{
		state->isEpsilonClosed = true;
		recursionSet.insert(state);
		#ifdef DEBUG_GRAMMAR
			CatLog::log("Epsilon closing state: ");
			CatLog::log(state->stateIndex);
			CatLog::log("\n");
		#endif
	}
	for (int i = 0; i < (int)currentState->transitions.size(); i++)
	{
		if (currentState->transitions[i].transitionToken->getType() == ProductionTokenType::Epsilon)
		{
			DFAState* transitionNextState = currentState->transitions[i].nextState;
			if (state == currentState)
			{
				state->transitions.erase(state->transitions.begin() + i);
				i--;
			}
			if (recursionSet.find(transitionNextState) == recursionSet.end())
			{
				recursionSet.insert(transitionNextState);
				epsilonClose(state, transitionNextState, recursionSet);
			}
		}
		else 
		{
			if (state != currentState)
			{
				state->addTransition(currentState->transitions[i].transitionToken, currentState->transitions[i].nextState, currentState->transitions[i].item);
			}
			if (!currentState->transitions[i].nextState->isEpsilonClosed)
			{
				std::set<DFAState*> recSet;
				epsilonClose(currentState->transitions[i].nextState, currentState->transitions[i].nextState, recSet);
			}
		}
	}
}


DFAState* SLRParser::addState(DFAState* state, std::vector<DFAState*>& nfa)
{
	for (unsigned int i = 0; i < nfa.size(); i++)
	{
		if (*nfa[i] == *state)
		{
			delete state;
			return nfa[i];
		}
	}
	state->stateIndex = nfa.size();
	state->reachable = false;
	state->isEpsilonClosed = false;
	nfa.push_back(state);
	return state;
}


void SLRParser::cleanupAfterConversionToDFA(std::vector<DFAState*>& dfaToClean)
{
	bool foundAggregationTarget = true;

	while (foundAggregationTarget)
	{
		foundAggregationTarget = false;
		for (unsigned int i = 0; i < dfaToClean.size(); i++)
		{
			DFAState* currentState = dfaToClean[i];
			for (unsigned int j = 0; j < currentState->transitions.size(); j++)
			{
				if (hasMultipleTransitionsWithToken(currentState, currentState->transitions[j].transitionToken))
				{
					aggregateTokenTransitionIntoNewState(currentState, currentState->transitions[j].transitionToken, dfaToClean, currentState->transitions[j].item);
					foundAggregationTarget = true;
					j = 0;
				}
			}
		}
	}
}


void SLRParser::markReachableStates(DFAState* currentState)
{
	currentState->reachable = true;
	for (unsigned int i = 0; i < currentState->transitions.size(); i++)
	{
		if (!currentState->transitions[i].nextState->reachable)
		{
			markReachableStates(currentState->transitions[i].nextState);
		}
	}
}


bool SLRParser::hasMultipleTransitionsWithToken(DFAState* state, ProductionToken* token) const
{
	int numTransitionsWithToken = 0;
	for (unsigned int i = 0; i < state->transitions.size(); i++)
	{
		if (*state->transitions[i].transitionToken == *token)
		{
			numTransitionsWithToken++;
		}
	}
	return numTransitionsWithToken > 1;
}


void SLRParser::aggregateTokenTransitionIntoNewState(DFAState* state, ProductionToken* token, std::vector<DFAState*>& nfa, const Item& item)
{
	DFAState* newState = new DFAState();
	
	for (int i = 0; i < (int)state->transitions.size(); i++)
	{
		if (*state->transitions[i].transitionToken == *token)
		{
			DFAState* aggregateState = state->transitions[i].nextState;
			state->transitions.erase(state->transitions.begin() + i);
			i--;

			for (unsigned int j = 0; j < aggregateState->items.size(); j++)
			{
				newState->addItem(aggregateState->items[j]);
			}
			for (unsigned int j = 0; j < aggregateState->transitions.size(); j++)
			{
				newState->addTransition(aggregateState->transitions[j].transitionToken, aggregateState->transitions[j].nextState, aggregateState->transitions[j].item);
			}
		}
	}
	newState = addState(newState, nfa);
	state->addTransition(token, newState, item);
}


bool SLRParser::tryReduce(DFAState* currentState, std::vector<StackItem*>& stack, StackItem* nextToken, RuntimeContext* context) const
{
	for (unsigned int i = 0; i < currentState->items.size(); i++)
	{
		const Item& item = currentState->items[i];
		//Check for possible reduce move
		//Reduce only if the next token is in the follow set of the production.
		if ((item.tokenOffset >= item.rule->getNumTokens()
			|| (item.rule->getNumTokens() == 1 && item.rule->getToken(0)->getType() == ProductionTokenType::Epsilon))
			&& isStackItemInFollowSet(nextToken, item.production->getFollowSet()))
		{
			#ifdef DEBUG_GRAMMAR
			if (nextToken->getProductionIfProduction())
			{
				CatLog::log("Reduce move state: ");
				CatLog::log(currentState->stateIndex);
				CatLog::log(" item: ");
				CatLog::log(item.toString().c_str());
				CatLog::log(" because ");
				CatLog::log(nextToken->getProductionIfProduction()->getProductionName());
				CatLog::log(" is in the follow set of ");
				CatLog::log(item.production->getProductionName());
				CatLog::log("\n");
			}
			else
			{
				CatLog::log("Reduce move state: ");
				CatLog::log(currentState->stateIndex);
				CatLog::log(" item: ");
				CatLog::log(item.toString().c_str());
				CatLog::log(" because ");
				CatLog::log(nextToken->getTokenIfToken()->getSubTypeName(nextToken->getTokenIfToken()->getTokenSubType()));
				CatLog::log(" is in the follow set of ");
				CatLog::log(item.production->getProductionName());
				CatLog::log("\n");
			}
			#endif
			
			StackItemProduction* reducedProduction = new StackItemProduction(item.production, item.rule);

			ASTNodeParser nodeParser(stack, item.rule->getNumTokens(), context);

			reducedProduction->astNode = item.rule->executeSemanticAction(nodeParser);
			std::size_t numTokensInRule = item.rule->getNumTokens();
			for (int j = (int)stack.size() - (int)numTokensInRule; j < (int)stack.size(); j++)
			{
				reducedProduction->addChildItem(stack[j]);
			}
			for (unsigned int j = 0; j < item.rule->getNumTokens(); j++)
			{
				stack.pop_back();
			}
			stack.push_back(reducedProduction);
			return true;
		}
	}
	return false;
}


DFAState* SLRParser::canShift(DFAState* currentState, StackItem* tokenToShift) const
{
	const ParseToken* token = tokenToShift->getTokenIfToken();
	const Production* production = tokenToShift->getProductionIfProduction();

	for (unsigned int i = 0; i < currentState->transitions.size(); i++)
	{
		const DFAState::DFAStateTransition& transition = currentState->transitions[i];
		//Check for possible reduce move
		//Reduce only if the next token is in the follow set of the production.
		ProductionTokenType transitionTokenType = transition.transitionToken->getType();

		if (transitionTokenType == ProductionTokenType::Terminal
			&& token != nullptr)
		{
			const ProductionTerminalToken* terminal = static_cast<const ProductionTerminalToken*>(transition.transitionToken); 
			if (terminal->getTokenId() == token->getTokenID()
				&& terminal->getTokenSubType() == token->getTokenSubType())
			{
				#ifdef DEBUG_GRAMMAR
					CatLog::log("Shift terminal, state: ");
					CatLog::log(currentState->stateIndex);
					CatLog::log(" terminal: ");
					CatLog::log(token->getSubTypeSymbol(token->getTokenSubType()));
					CatLog::log("\n");
				#endif
				return transition.nextState;
			}
		}
		else if (transitionTokenType == ProductionTokenType::NonTerminal
				 && production != nullptr)
		{
			const ProductionNonTerminalToken* nonTerminal = static_cast<const ProductionNonTerminalToken*>(transition.transitionToken);
			if (nonTerminal->getProduction()->getProductionID() == production->getProductionID())
			{
				#ifdef DEBUG_GRAMMAR
					CatLog::log("Shift non-terminal, state: ");
					CatLog::log(currentState->stateIndex);
					CatLog::log(" production: ");
					CatLog::log(production->getProductionName());
					CatLog::log("\n");
				#endif
				return transition.nextState;
			}
		}
	}
	return nullptr;
}


bool SLRParser::isStackItemInFollowSet(StackItem* item, ProductionTokenSet* followSet) const
{
	const ParseToken* token = item->getTokenIfToken();
	if (token != nullptr)
	{
		return followSet->isInSet(token);
	}
	else
	{
		const Production* production = item->getProductionIfProduction();
		if (production != nullptr)
		{
			return followSet->isInSet(production);
		}
	}
	//We should never get here
	assert(false);
	return false;
}


void SLRParser::printStack(const std::vector<StackItem*> stack) const
{
	for (unsigned int i = 0; i < stack.size(); i++)
	{
		const ParseToken* token = stack[i]->getTokenIfToken();
		if (token != nullptr)
		{
			CatLog::log(token->getSubTypeSymbol(token->getTokenSubType()));
		}
		else 
		{
			const Production* production = stack[i]->getProductionIfProduction();
			if (production != nullptr)
			{
				CatLog::log(production->getProductionName());
			}
		}
	}
}


void SLRParser::scanForConflicts() const
{
	CatLog::log("\n##########################\n");
	CatLog::log("# Checking for conflicts #\n");
	CatLog::log("##########################\n\n");

	//Check for shift/reduce conflicts
	bool foundShiftReduceConflict = false;
	for (int i = 0; i < (int)dfa.size(); i++)
	{
		DFAState* state = dfa[i];
		int conflictCount = 0;
		for (int j = 0; j < (int)state->transitions.size(); j++)
		{
			ProductionToken* transitionToken = state->transitions[j].transitionToken;
			for (int k = 0; k < (int)state->items.size(); k++)
			{
				const Item& item = state->items[k];
				if ((item.tokenOffset >= item.rule->getNumTokens()
					|| (item.rule->getNumTokens() == 1 && item.rule->getToken(0)->getType() == ProductionTokenType::Epsilon))
					&& item.production->getFollowSet()->isInSet(transitionToken))
				{
					conflictCount++;
					if (!foundShiftReduceConflict)
					{
						foundShiftReduceConflict = true;
						CatLog::log("Shift reduce conflicts:\n\n");
					}
					if (conflictCount == 1)
					{
						CatLog::log("State: ");
						CatLog::log(state->stateIndex);
						CatLog::log(" on input: ");
						CatLog::log(transitionToken->getSymbol());
						CatLog::log("\n");
						if (state->transitions[j].item.production == nullptr)
						{
							CatLog::log("Shift: epsilon\n");
						}
						else
						{
							CatLog::log("Shift: ");
							CatLog::log(state->transitions[j].item.toString().c_str());
							CatLog::log("\n");
						}
						CatLog::log("Reduce: ");
						CatLog::log(item.toString().c_str());
						CatLog::log("\n");
					}
				}
			}
		}
		if (conflictCount > 1)
		{
			CatLog::log("And ");
			CatLog::log(conflictCount - 1);
			CatLog::log(" other shift-reduce conflict(s) for this state. \n\n");
		}
		else if (conflictCount == 1)
		{
			CatLog::log("\n");
		}
	}
	if (!foundShiftReduceConflict)
	{
		CatLog::log("No shift-reduce conflicts.\n");
	}

	//Check for reduce/reduce conflicts
	bool foundReduceReduceConflict = false;
	for (int i = 0; i < (int)dfa.size(); i++)
	{
		DFAState* state = dfa[i];
		for (int j = 0; j < (int)state->items.size() - 1; j++)
		{
			const Item& firstItem = state->items[j];
			const Production* firstItemProduction = firstItem.production;
			if (firstItem.tokenOffset >= firstItem.rule->getNumTokens()
				|| (firstItem.rule->getNumTokens() == 1 && firstItem.rule->getToken(0)->getType() == ProductionTokenType::Epsilon))
			{
				for (int k = j + 1; k < (int)state->items.size(); k++)
				{
					const Item& secondItem = state->items[k];
					const Production* secondItemProduction = secondItem.production;
					if ((secondItem.tokenOffset >= secondItem.rule->getNumTokens()
						 || (secondItem.rule->getNumTokens() == 1 && secondItem.rule->getToken(0)->getType() == ProductionTokenType::Epsilon))
						&& firstItemProduction->getFollowSet()->overlaps(secondItemProduction->getFollowSet()))
					{
						if (!foundReduceReduceConflict)
						{
							foundReduceReduceConflict = true;
							CatLog::log("Reduce reduce conflicts:\n\n");
						}
						CatLog::log("State: ");
						CatLog::log(state->stateIndex);
						CatLog::log("\n");
						CatLog::log("First: ");
						CatLog::log(firstItem.toString().c_str());
						CatLog::log("\n");
						CatLog::log("Second: ");
						CatLog::log(secondItem.toString().c_str());
						CatLog::log("\n\n");
					}
				}
			}
		}
	}
	if (!foundReduceReduceConflict)
	{
		CatLog::log("No reduce-reduce conflicts.\n");
	}
	if (!foundReduceReduceConflict && ! foundShiftReduceConflict)
	{
		CatLog::log("\nGrammar is SLR.\n\n");
	}
	else
	{
		CatLog::log("\nGrammar is not SLR.\n\n");
	}
}


SLRParseResult* SLRParser::parse(const std::vector<ParseToken*>& tokens, int whiteSpaceTokenID, int commentTokenID, RuntimeContext* context) const
{
	SLRParseResult* parseResult = new SLRParseResult();

	std::vector<StackItem*> parseStack;
	int tokenIndex = 0;
	DFAState* currentState = dfa[0];

	StackItem* startingStackItem = new StackItem();
	startingStackItem->state = currentState;
	parseStack.push_back(startingStackItem);

	const Production* finalProduction = currentState->items[0].production;
	#ifdef DEBUG_GRAMMAR
		CatLog::log("###############\n");
		CatLog::log("Beginning parse\n");
		CatLog::log("###############\n");
	#endif
	while (true)
	{
		#ifdef DEBUG_GRAMMAR
			CatLog::log("State ");
			CatLog::log(currentState->stateIndex);
			CatLog::log(" Stack: ");
			printStack(parseStack);
			CatLog::log("\n\n");	
		#endif
		//Skip whitespace
		while (tokenIndex < (int)tokens.size()
			   && (tokens[tokenIndex]->getTokenID() == whiteSpaceTokenID
			       || tokens[tokenIndex]->getTokenID() == commentTokenID))
		{
			tokenIndex++;
		}
		StackItemToken* token = nullptr;
		if (tokenIndex < (int)tokens.size())
		{
			token = new StackItemToken(tokens[tokenIndex]);
			token->state = nullptr;
		}
		bool reduced = tryReduce(currentState, parseStack, token, context);
		if (reduced)
		{
			delete token;
			if (parseStack.size() > 1)
			{
				std::size_t previousStackItem = parseStack.size() - 2;
				DFAState* state = parseStack[previousStackItem]->state;
				DFAState* newState = canShift(state, parseStack[parseStack.size() - 1]);
				if (newState != nullptr)
				{
					parseStack[parseStack.size() - 1]->state = newState;
					currentState = newState;
				}
				else if (parseStack.size() == 2
						 && parseStack[1]->getProductionIfProduction() != nullptr
						 && parseStack[1]->getProductionIfProduction()->getProductionID() == finalProduction->getProductionID())
				{
					//Succesful parse
					parseResult->success = true;
					parseResult->astRootNode = parseStack[1]->astNode;
					Tools::deleteElements(parseStack);
					return parseResult;
				}
				else
				{
					Tools::deleteElements(parseStack);
					return parseResult;
				}
			}
			else
			{
				Tools::deleteElements(parseStack);
				//Should never happen
				assert(false);
				return parseResult;
			}
		}
		else
		{
			//Check for shift move
			DFAState* newState = canShift(currentState, token);
			if (newState != nullptr)
			{
				token->state = newState;
				currentState = newState;
				parseStack.push_back(token);
				tokenIndex++;
			}
			else
			{
				if (token != nullptr)
				{
					if (token->getTokenIfToken())
					{
						const Lexeme* errorLexeme = token->getTokenIfToken()->getLexeme();
						std::string errorToken = std::string(errorLexeme->getDataPointer(), errorLexeme->length);
						if (errorToken == "")
						{
							errorToken = "end of line";
						}
						parseResult->errorMessage = std::string("Did not expect ") + errorToken + " here.";
						parseResult->errorPosition = errorLexeme->offset;
					}
					else if (token->getProductionIfProduction())
					{
						const Production* errorProduction = token->getProductionIfProduction();
						parseResult->errorMessage = std::string("Did not expect ") + errorProduction->getProductionName() + " here.";
					}
					delete token;
				}
				Tools::deleteElements(parseStack);
				return parseResult;
			}
			
		}
	}
	Tools::deleteElements(parseStack);
	return parseResult;
}


SLRParser::~SLRParser()
{
	for (int i = 0; i < (int)dfa.size(); i++)
	{
		delete dfa[i];
	}
}