/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/AutoCompletion.h"
#include "jitcat/CatBuiltInFunctionCall.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatTokenizer.h"
#include "jitcat/CommentToken.h"
#include "jitcat/ConstantToken.h"
#include "jitcat/CustomTypeInfo.h"
#include "jitcat/Document.h"
#include "jitcat/ErrorToken.h"
#include "jitcat/IdentifierToken.h"
#include "jitcat/Lexeme.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/MemberFunctionInfo.h"
#include "jitcat/OneCharToken.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/WhitespaceToken.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <set>
#include <sstream>

using namespace jitcat;
using namespace jitcat::Reflection;
using namespace jitcat::Tokenizer;


std::vector<AutoCompletion::AutoCompletionEntry> AutoCompletion::autoComplete(const std::string& expression, std::size_t cursorPosition, CatRuntimeContext* context)
{
	Document doc(expression.c_str(), expression.size());

	CatTokenizer tokenizer;
	
	tokenizer.tokenize(doc);
	auto tokens = doc.getTokens();
	int startingTokenIndex = findStartTokenIndex(doc, (int)cursorPosition - 1, tokens);

	std::string expressionTailEnd = "";
	std::vector<std::pair<const ParseToken*, bool>> subExpression = getSubExpressionToAutoComplete(tokens, startingTokenIndex, expressionTailEnd);

	std::vector<AutoCompletion::AutoCompletionEntry> results;
	int last = (int)subExpression.size() - 1;
	TypeMemberInfo* currentMemberInfo = nullptr;
	MemberFunctionInfo* currentFunctionInfo = nullptr;
	bool foundValidAutoCompletion = false;
	bool autoCompleteOnArray = false;
	if (last >= 0)
	{
		for (unsigned int i = 0; i <= (unsigned int)last; i++)
		{
			std::string lowercaseIdentifier;
			std::size_t identifierOffset;
			if (subExpression[i].first != nullptr)
			{
				lowercaseIdentifier = Tools::toLowerCase(subExpression[i].first->lexeme);
				identifierOffset = subExpression[i].first->lexeme.data() - doc.getDocumentData().c_str();
				autoCompleteOnArray = subExpression[i].second;
			}
			else if (i > 0)
			{
				identifierOffset = (subExpression[i - 1].first->lexeme.data() - doc.getDocumentData().c_str()) + subExpression[i - 1].first->lexeme.length();
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
						addOptionsFromGlobalScope(memberPrefix, expression, expressionTailEnd, completionOffset, context, results);
					}
				}
				else if (currentMemberInfo != nullptr)
				{
					autoCompleteOnType(currentMemberInfo->getType(), autoCompleteOnArray, memberPrefix, expression, completionOffset, expressionTailEnd, results);
				}
				else if (currentFunctionInfo != nullptr && currentFunctionInfo->getReturnType().removeIndirection().isReflectableObjectType())
				{
					autoCompleteOnType(currentFunctionInfo->getReturnType(), autoCompleteOnArray, memberPrefix, expression, completionOffset, expressionTailEnd, results);
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
				if (!autoCompleteOnArray || currentMemberInfo == nullptr)
				{
					if (currentMemberInfo == nullptr)
					{
						currentFunctionInfo = context->findFirstMemberFunction(lowercaseIdentifier, scopeId);
					}
				}
				else if (currentMemberInfo->getType().removeIndirection().isReflectableObjectType())
				{
					TypeInfo* objectType = currentMemberInfo->getType().removeIndirection().getObjectType();
					auto arrayIndices = objectType->getMemberFunctionsByName("[]");
					if (arrayIndices.size() > 0)
					{
						currentMemberInfo = nullptr;
						currentFunctionInfo = arrayIndices[0];
					}
				}
			}
			else if (currentMemberInfo != nullptr && currentMemberInfo->getType().removeIndirection().isReflectableObjectType())
			{
				if (!traverseType(currentMemberInfo->getType(), currentMemberInfo, currentFunctionInfo, lowercaseIdentifier, autoCompleteOnArray))
				{
					break;
				}
			}
			else if (currentFunctionInfo != nullptr && currentFunctionInfo->getReturnType().removeIndirection().isReflectableObjectType())
			{
				if (!traverseType(currentFunctionInfo->getReturnType(), currentMemberInfo, currentFunctionInfo, lowercaseIdentifier, autoCompleteOnArray))
				{
					break;
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
		addOptionsFromGlobalScope("", expression, expressionTailEnd, cursorPosition, context, results);
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


std::vector<std::pair<const ParseToken*, bool>> AutoCompletion::getSubExpressionToAutoComplete(const std::vector<ParseToken>& tokens, int startingTokenIndex, std::string& expressionTailEnd)
{
	bool readTailEnd = false;
	//Tokenize the entire expression, then find the token at the cursorPosition, then backtrack from there to find the 
	//list of consecutive member dereferences/scopes
	if (startingTokenIndex < 0)
	{
		return std::vector<std::pair<const ParseToken*, bool>>();
	}
	const ParseToken* startingToken = &tokens[(unsigned int)startingTokenIndex];
	std::vector<std::pair<const ParseToken*, bool>> subExpressions;
	if (startingToken->tokenID == CatTokenizer::identifier
		|| (startingToken->tokenID == CatTokenizer::oneChar 
		   && startingToken->subType == static_cast<typename std::underlying_type<OneChar>::type>(OneChar::Dot)))
	{
		int currentUnmatchedCloseBrackets = 0;
		int currentUnmatchedCloseParenthesis = 0;
		int totalOpenBrackets = 0;

		bool backtrackingDone = false;
		for (int i = startingTokenIndex; i >= 0 && !backtrackingDone; i--)
		{
			if (!readTailEnd)
			{
				if (tokens[i].tokenID == CatTokenizer::identifier && currentUnmatchedCloseBrackets == 0 && currentUnmatchedCloseParenthesis == 0)
				{
					readTailEnd = true;
				}
				else
				{
					expressionTailEnd = Tools::append(tokens[i].lexeme, expressionTailEnd);
				}
			}
			if (tokens[i].tokenID == CatTokenizer::oneChar)
			{
				switch (static_cast<OneChar>(tokens[i].subType))
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
						totalOpenBrackets++;
						break;
					case OneChar::BracketClose:
						currentUnmatchedCloseBrackets++;
						break;
					case OneChar::Dot:
						if (i == startingTokenIndex)
						{
							subExpressions.push_back(std::make_pair((const Tokenizer::ParseToken*)nullptr, false));
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
			else if (tokens[i].tokenID == CatTokenizer::identifier)
			{
				subExpressions.push_back(std::make_pair(&tokens[i], totalOpenBrackets > 0));
				totalOpenBrackets = 0;
			}
			else if (tokens[i].tokenID == CatTokenizer::whiteSpace
					 || tokens[i].tokenID == CatTokenizer::comment)
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


void AutoCompletion::addOptionsFromGlobalScope(const std::string& prefix, const std::string& expression, const std::string& expressionTailEnd, 
											   std::size_t completionOffset, CatRuntimeContext* context, std::vector<AutoCompletionEntry>& entries)
{
	//search in dynamic scopes of the runtime context
	for (int i = context->getNumScopes() - 1; i >= 0; i--)
	{
		TypeInfo* typeInfo = context->getScopeType((CatScopeID)i);
		if (typeInfo != nullptr)
		{
			addOptionsFromTypeInfo(typeInfo, entries, prefix, expression, completionOffset, expressionTailEnd);
		}
	}

	//search in static scopes of the runtime context
	for (int i = InvalidScopeID - context->getNumStaticScopes(); i < InvalidScopeID; ++i)
	{
		TypeInfo* typeInfo = context->getScopeType((CatScopeID)i);
		if (typeInfo != nullptr)
		{
			addOptionsFromTypeInfo(typeInfo, entries, prefix, expression, completionOffset, expressionTailEnd);
		}
	}

	addOptionsFromBuiltIn(entries, prefix, expression, completionOffset);
}


int AutoCompletion::findStartTokenIndex(const Document& doc, int cursorPosition, const std::vector<ParseToken>& tokens)
{
	for (int i = 0; i < (int)tokens.size(); i++)
	{
		const ParseToken* token = &tokens[i];
		if (token != nullptr)
		{
			const Lexeme& lexeme = token->lexeme;
			std::size_t lexemeOffset = doc.getOffsetInDocument(lexeme);
			if ((int)lexemeOffset <= cursorPosition
				&& (int)(lexemeOffset + lexeme.length()) > cursorPosition)
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
				std::string replacement = iter.second->getMemberName();
				int numberOfCharactersToAdd = (int)replacement.size();
				newExpression.replace(prefixOffset, lowercasePrefix.size(), replacement);
				results.push_back(AutoCompletionEntry(newExpression, iter.second->getMemberName(), findLocation == 0, prefixOffset + numberOfCharactersToAdd));
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
				newExpression.replace(prefixOffset, lowercasePrefix.size(), iter.second->getMemberFunctionName() + parenthesesToAdd);
				results.push_back(AutoCompletionEntry(newExpression, iter.second->getMemberFunctionName()  + parenthesesToAdd, findLocation == 0, prefixOffset + iter.second->getMemberFunctionName().size() + parenthesesToAdd.size()));
			}
		}
	}
}


void AutoCompletion::addOptionsFromBuiltIn(std::vector<AutoCompletion::AutoCompletionEntry>& results, const std::string& lowercasePrefix, 
										   const std::string& originalExpression, std::size_t prefixOffset)
{
	auto& allFunctions = jitcat::AST::CatBuiltInFunctionCall::getAllBuiltInFunctions();
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


bool AutoCompletion::isGlobalScopeAutoCompletable(const std::vector<ParseToken>& tokens, int startingTokenIndex)
{
	if (startingTokenIndex < 0)
	{
		return true;
	}
	const ParseToken* startingToken = &tokens[startingTokenIndex];
	while (startingToken->tokenID == CatTokenizer::whiteSpace)
	{
		startingTokenIndex--;
		if (startingTokenIndex < 0)
		{
			return true;
		}
		else
		{
			startingToken = &tokens[startingTokenIndex];
		}
	}
	//There are several types of token after which it does not make sense to do any autocompletion
	if (startingToken->tokenID == CatTokenizer::constant
		|| startingToken->tokenID == CatTokenizer::error
		|| startingToken->tokenID == CatTokenizer::identifier
		|| startingToken->tokenID == CatTokenizer::comment
		|| (startingToken->tokenID == CatTokenizer::oneChar
			&& (startingToken->subType == (unsigned short)OneChar::BracketClose
				|| startingToken->subType == (unsigned short)OneChar::ParenthesesClose
				|| startingToken->subType == (unsigned short)OneChar::Dot)))
	{
		return false;
	}
	return true;
}


bool AutoCompletion::autoCompleteOnType(const CatGenericType& type, bool completeOnArrayIndex, const std::string& memberPrefix, 
										const std::string& expression, std::size_t completionOffset, const std::string& expressionTailEnd, 
										 std::vector<AutoCompletion::AutoCompletionEntry>& results)
{
	const CatGenericType& nakedType = type.removeIndirection();
	if (!nakedType.isReflectableObjectType())
	{
		return false;
	}
	TypeInfo* typeInfo = nakedType.getObjectType();
	if (completeOnArrayIndex && typeInfo->getMemberFunctionsByName("[]").size() > 0)
	{
		MemberFunctionInfo* memberFunctionInfo = typeInfo->getMemberFunctionsByName("[]")[0];
		const CatGenericType& returnType = memberFunctionInfo->getReturnType().removeIndirection();
		if (returnType.isReflectableObjectType())
		{
			addOptionsFromTypeInfo(returnType.getObjectType(), results, memberPrefix, expression, completionOffset, expressionTailEnd);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		addOptionsFromTypeInfo(typeInfo, results, memberPrefix, expression, completionOffset, expressionTailEnd);
		return true;
	}
}


bool AutoCompletion::traverseType(const CatGenericType& type, Reflection::TypeMemberInfo*& currentMemberInfo, Reflection::MemberFunctionInfo*& currentFunctionInfo, 
								  const std::string& lowerCaseIdentifier, bool traverseOnArrayIndex)
{
	if (type.removeIndirection().isReflectableObjectType())
	{
		TypeInfo* objectType = type.removeIndirection().getObjectType();
		if (traverseOnArrayIndex)
		{
			auto arrayIndices = objectType->getMemberFunctionsByName("[]");
			if (arrayIndices.size() > 0)
			{
				return traverseType(arrayIndices[0]->getReturnType(), currentMemberInfo, currentFunctionInfo, lowerCaseIdentifier, false);
			}
		}
		else
		{
			currentMemberInfo = objectType->getMemberInfo(lowerCaseIdentifier);
			if (currentMemberInfo == nullptr)
			{
				currentFunctionInfo = objectType->getFirstMemberFunctionInfo(lowerCaseIdentifier);

				if (currentFunctionInfo == nullptr)
				{
					//failed
					return false;
				}
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}


AutoCompletion::AutoCompletionEntry::AutoCompletionEntry(const std::string& newExpression, const std::string& autoCompletionValue, bool isPrefixSuggestion, std::size_t newCursorPosition):
	newExpression(newExpression),
	autoCompletionValue(autoCompletionValue),
	isPrefixSuggestion(isPrefixSuggestion),
	newCursorPosition(newCursorPosition)
{
}
