/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "AutoCompletion.h"
#include "CatFunctionCall.h"
#include "CatRuntimeContext.h"
#include "CatTokenizer.h"
#include "CommentToken.h"
#include "ConstantToken.h"
#include "CustomTypeInfo.h"
#include "Document.h"
#include "ErrorToken.h"
#include "IdentifierToken.h"
#include "Lexeme.h"
#include "OneCharToken.h"
#include "Tools.h"
#include "TypeInfo.h"
#include "WhitespaceToken.h"

#include <algorithm>
#include <functional>
#include <set>
#include <sstream>


std::vector<AutoCompletion::AutoCompletionEntry> AutoCompletion::autoComplete(const std::string& expression, std::size_t cursorPosition, CatRuntimeContext* context)
{
	Document doc(expression.c_str(), expression.size());

	CatTokenizer tokenizer;
	std::vector<ParseToken*> tokens;
	tokenizer.tokenize(&doc, tokens, nullptr);
	int startingTokenIndex = findStartTokenIndex((int)cursorPosition - 1, tokens);
	while (startingTokenIndex >= 0 && tokens[(unsigned int)startingTokenIndex] == nullptr)
	{
		startingTokenIndex--;
	}
	std::string expressionTailEnd = "";
	std::vector<IdentifierToken*> subExpression = getSubExpressionToAutoComplete(tokens, startingTokenIndex, expressionTailEnd);

	std::vector<AutoCompletion::AutoCompletionEntry> results;
	int last = (int)subExpression.size() - 1;
	TypeMemberInfo* currentMemberInfo = nullptr;
	MemberFunctionInfo* currentFunctionInfo = nullptr;
	bool foundValidAutoCompletion = false;
	if (last >= 0)
	{
		for (unsigned int i = 0; i <= (unsigned int)last; i++)
		{
			std::string lowercaseIdentifier;
			std::size_t identifierOffset;
			if (subExpression[i] != nullptr)
			{
				lowercaseIdentifier = Tools::toLowerCase(subExpression[i]->getLexeme()->toString());
				identifierOffset = subExpression[i]->getLexeme()->offset;
			}
			else if (i > 0)
			{
				identifierOffset = subExpression[i - 1]->getLexeme()->offset + subExpression[i - 1]->getLexeme()->length;
			}
			else
			{
				break;
			}
			if (i == (unsigned int)last)
			{
				std::string memberPrefix = lowercaseIdentifier;
				foundValidAutoCompletion = true;
				std::size_t completionOffset = identifierOffset + expressionTailEnd.size();
				if (currentMemberInfo == nullptr && currentFunctionInfo == nullptr)
				{
					if (i == 0)
					{
						//search in the runtime context
						for (int i = context->getNumScopes() - 1; i >= 0; i--)
						{
							TypeInfo* typeInfo = context->getScopeType((CatScopeID)i);
							if (typeInfo != nullptr)
							{
								addOptionsFromTypeInfo(typeInfo, results, memberPrefix, expression, completionOffset, expressionTailEnd);
							}
						}
						addOptionsFromBuiltIn(results, memberPrefix, expression, completionOffset);
					}
				}
				else if (currentMemberInfo != nullptr && currentMemberInfo->catType == CatType::Object)
				{
					addOptionsFromTypeInfo(currentMemberInfo->nestedType, results, memberPrefix, expression, completionOffset, expressionTailEnd);
				}
				else if (currentFunctionInfo != nullptr && currentFunctionInfo->returnType.getCatType() == CatType::Object)
				{
					addOptionsFromTypeInfo(currentFunctionInfo->returnType.getObjectType(), results, memberPrefix, expression, completionOffset, expressionTailEnd);
				}
				else
				{
					//Failed
				}
			}
			else if (currentMemberInfo == nullptr && currentFunctionInfo == nullptr)
			{
				CatScopeID scopeId;
				currentMemberInfo = context->findVariable(lowercaseIdentifier, scopeId);
				if (currentMemberInfo == nullptr)
				{
					currentFunctionInfo = context->findFunction(lowercaseIdentifier, scopeId);
				}
			}
			else if (currentMemberInfo != nullptr && currentMemberInfo->catType == CatType::Object)
			{
				TypeMemberInfo* currentMember = currentMemberInfo;
				currentMemberInfo = currentMemberInfo->nestedType->getMemberInfo(lowercaseIdentifier);
				if (currentMemberInfo == nullptr)
				{
					currentFunctionInfo = currentMember->nestedType->getMemberFunctionInfo(lowercaseIdentifier);
					if (currentFunctionInfo == nullptr)
					{
						//failed
						break;
					}
				}
			}
			else if (currentFunctionInfo != nullptr && currentFunctionInfo->returnType.getCatType() == CatType::Object)
			{
				MemberFunctionInfo* currentFunction = currentFunctionInfo;
				currentFunctionInfo = currentFunctionInfo->returnType.getObjectType()->getMemberFunctionInfo(lowercaseIdentifier);
				if (currentFunctionInfo == nullptr)
				{
					currentMemberInfo = currentFunction->returnType.getObjectType()->getMemberInfo(lowercaseIdentifier);
					if (currentMemberInfo == nullptr)
					{
						//failed
						break;
					}
				}
			}
			else
			{
				//failed
				break;
			}
		}
	}
	if (!foundValidAutoCompletion && isGlobalScopeAutoCompletable(tokens, startingTokenIndex))
	{
		//search in the runtime context
		for (int i = context->getNumScopes() - 1; i >= 0; i--)
		{
			TypeInfo* typeInfo = context->getScopeType((CatScopeID)i);
			if (typeInfo != nullptr)
			{
				addOptionsFromTypeInfo(typeInfo, results, "", expression, cursorPosition, expressionTailEnd);
			}
		}
		addOptionsFromBuiltIn(results, "", expression, cursorPosition);
	}
	std::sort(std::begin(results), std::end(results), [](const AutoCompletion::AutoCompletionEntry& a, const AutoCompletion::AutoCompletionEntry& b) 
		{
			if (a.isPrefixSuggestion == b.isPrefixSuggestion)
			{
				return Tools::toLowerCase(a.autoCompletionValue).compare(Tools::toLowerCase(b.autoCompletionValue)) < 0;
			}
			else
			{
				return a.isPrefixSuggestion && !b.isPrefixSuggestion; 
			}
		});
	return results;
}


std::vector<IdentifierToken*> AutoCompletion::getSubExpressionToAutoComplete(const std::vector<ParseToken*>& tokens, int startingTokenIndex, std::string& expressionTailEnd)
{
	bool readTailEnd = false;
	//Tokenize the entire expression, then find the token at the cursorPosition, then backtrack from there to find the 
	//list of consecutive member dereferences/scopes
	if (startingTokenIndex < 0)
	{
		return std::vector<IdentifierToken*>();
	}
	ParseToken* startingToken = tokens[(unsigned int)startingTokenIndex];
	std::vector<IdentifierToken*> subExpressions;
	if (startingToken->getTokenID() == IdentifierToken::getID()
		|| (startingToken->getTokenID() == OneCharToken::getID() 
		   && startingToken->getTokenSubType() == static_cast<typename std::underlying_type<OneChar>::type>(OneChar::Dot)))
	{
		int currentUnmatchedCloseBrackets = 0;
		int currentUnmatchedCloseParenthesis = 0;
		bool skipping = false;

		bool backtrackingDone = false;
		for (int i = startingTokenIndex; i >= 0 && !backtrackingDone; i--)
		{
			if (!readTailEnd)
			{
				if (tokens[i]->getTokenID() == IdentifierToken::getID() && currentUnmatchedCloseBrackets == 0 && currentUnmatchedCloseParenthesis == 0)
				{
					readTailEnd = true;
				}
				else
				{
					expressionTailEnd = tokens[i]->getLexeme()->toString() + expressionTailEnd;
				}
			}
			if (tokens[i]->getTokenID() == OneCharToken::getID())
			{
				switch (static_cast<OneChar>(tokens[i]->getTokenSubType()))
				{
					case OneChar::ParenthesesOpen:
						currentUnmatchedCloseParenthesis--;
						if (currentUnmatchedCloseParenthesis < 0)
						{
							backtrackingDone = true;
						}
						break;
					case OneChar::ParenthesesClose:
						currentUnmatchedCloseParenthesis++;
						break;
					case OneChar::BracketOpen:
						currentUnmatchedCloseBrackets--;
						if (currentUnmatchedCloseBrackets < 0)
						{
							backtrackingDone = true;
						}
						break;
					case OneChar::BracketClose:
						currentUnmatchedCloseBrackets++;
						break;
					case OneChar::Dot:
						if (i == startingTokenIndex)
						{
							subExpressions.push_back(nullptr);
						}
						continue;
						break;
					default:
						if (currentUnmatchedCloseBrackets == 0 && currentUnmatchedCloseParenthesis == 0)
						{
							backtrackingDone = true;
						}
						break;
				}
			}
			else if (currentUnmatchedCloseBrackets > 0 || currentUnmatchedCloseParenthesis > 0)
			{
				//skip
				continue;
			}
			else if (tokens[i]->getTokenID() == IdentifierToken::getID())
			{
				subExpressions.push_back(static_cast<IdentifierToken*>(tokens[i]));
			}
			else if (tokens[i]->getTokenID() == WhitespaceToken::getID()
					 || tokens[i]->getTokenID() == CommentToken::getID())
			{
				//ignore whitespace and comments
				continue;
			}
			else if (currentUnmatchedCloseBrackets == 0 && currentUnmatchedCloseParenthesis == 0)
			{
				backtrackingDone = true;
			}
		}
		std::reverse(subExpressions.begin(), subExpressions.end());
	}
	return subExpressions;
}


int AutoCompletion::findStartTokenIndex(int cursorPosition, const std::vector<ParseToken*>& tokens)
{
	for (int i = 0; i < (int)tokens.size(); i++)
	{
		ParseToken* token = tokens[i];
		if (token != nullptr)
		{
			const Lexeme* lexeme = token->getLexeme();
			if ((int)lexeme->offset <= cursorPosition
				&& (int)(lexeme->offset + lexeme->length) > cursorPosition)
			{
				return i;
			}
		}
		else
		{
			return -1;
		}
	}
	return -1;
}


void AutoCompletion::addOptionsFromTypeInfo(TypeInfo* typeInfo, std::vector<AutoCompletion::AutoCompletionEntry>& results, 
											const std::string& lowercasePrefix, const std::string& originalExpression, std::size_t prefixOffset, const std::string& expressionTailEnd)
{
	if (typeInfo != nullptr)
	{
		const auto& members = typeInfo->getMembers();
		const auto& memberFunctions = typeInfo->getMemberFunctions();
		for (const auto& iter : members)
		{
			std::size_t findLocation = iter.first.find(lowercasePrefix);
			if (findLocation != iter.first.npos)
			{
				std::string newExpression = originalExpression;
				std::string replacement = iter.second->memberName;
				int numberOfCharactersToAdd = (int)replacement.size();
				if (expressionTailEnd.size() == 0 && iter.second->specificType == SpecificMemberType::ContainerType)
				{
					numberOfCharactersToAdd++;
					replacement += "[";
				}
				newExpression.replace(prefixOffset, lowercasePrefix.size(), replacement);
				results.push_back(AutoCompletionEntry(newExpression, iter.second->memberName, findLocation == 0, prefixOffset + numberOfCharactersToAdd));
			}
		}
		for (const auto& iter : memberFunctions)
		{
			std::size_t findLocation = iter.first.find(lowercasePrefix);
			if (findLocation != iter.first.npos)
			{
				std::string newExpression = originalExpression;
				std::string parenthesesToAdd = "(";
				if (iter.second->getNumberOfArguments() == 0)
				{
					parenthesesToAdd = "()";
				}
				newExpression.replace(prefixOffset, lowercasePrefix.size(), iter.second->memberFunctionName + parenthesesToAdd);
				results.push_back(AutoCompletionEntry(newExpression, iter.second->memberFunctionName  + parenthesesToAdd, findLocation == 0, prefixOffset + iter.second->memberFunctionName.size() + parenthesesToAdd.size()));
			}
		}
	}
}


void AutoCompletion::addOptionsFromBuiltIn(std::vector<AutoCompletion::AutoCompletionEntry>& results, const std::string& lowercasePrefix, 
										   const std::string& originalExpression, std::size_t prefixOffset)
{
	auto& allFunctions = CatFunctionCall::getAllBuiltInFunctions();
	for (auto& iter : allFunctions)
	{
		addIfPartialMatch(iter + "(", results, lowercasePrefix, originalExpression, prefixOffset);
	}
	//True and false constants
	addIfPartialMatch("false", results, lowercasePrefix, originalExpression, prefixOffset);
	addIfPartialMatch("true", results, lowercasePrefix, originalExpression, prefixOffset);
}


void AutoCompletion::addIfPartialMatch(const std::string& text, std::vector<AutoCompletion::AutoCompletionEntry>& results, 
									   const std::string& lowercasePrefix, const std::string& originalExpression, std::size_t prefixOffset)
{
	std::string lowerCase = Tools::toLowerCase(text);
	std::size_t findLocation = lowerCase.find(lowercasePrefix);
	if (findLocation != lowerCase.npos)
	{
		std::string newExpression = originalExpression;
		newExpression.replace(prefixOffset, lowercasePrefix.size(), text);
		results.push_back(AutoCompletionEntry(newExpression, text, findLocation == 0, prefixOffset + (int)text.size()));
	}
}


bool AutoCompletion::isGlobalScopeAutoCompletable(const std::vector<ParseToken*>& tokens, int startingTokenIndex)
{
	if (startingTokenIndex < 0)
	{
		return true;
	}
	ParseToken* startingToken = tokens[startingTokenIndex];
	while (startingToken->getTokenID() == WhitespaceToken::getID())
	{
		startingTokenIndex--;
		if (startingTokenIndex < 0)
		{
			return true;
		}
		else
		{
			startingToken = tokens[startingTokenIndex];
		}
	}
	//There are several types of token after which it does not make sense to do any autocompletion
	if (startingToken->getTokenID() == ConstantToken::getID()
		|| startingToken->getTokenID() == ErrorToken::getID()
		|| startingToken->getTokenID() == IdentifierToken::getID()
		|| startingToken->getTokenID() == CommentToken::getID()
		|| (startingToken->getTokenID() == OneCharToken::getID()
			&& (startingToken->getTokenSubType() == (int)OneChar::BracketClose
				|| startingToken->getTokenSubType() == (int)OneChar::ParenthesesClose
				|| startingToken->getTokenSubType() == (int)OneChar::Dot)))
	{
		return false;
	}
	return true;
}


AutoCompletion::AutoCompletionEntry::AutoCompletionEntry(const std::string& newExpression, const std::string& autoCompletionValue, bool isPrefixSuggestion, std::size_t newCursorPosition):
	newExpression(newExpression),
	autoCompletionValue(autoCompletionValue),
	isPrefixSuggestion(isPrefixSuggestion),
	newCursorPosition(newCursorPosition)
{
}
