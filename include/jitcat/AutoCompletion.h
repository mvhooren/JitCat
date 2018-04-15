/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatRuntimeContext;
class Document;
class IdentifierToken;
class ParseToken;
class TypeInfo;
#include <string>
#include <vector>


class AutoCompletion
{
private:
	AutoCompletion() {};
public:
	struct AutoCompletionEntry
	{
		AutoCompletionEntry(const std::string& newExpression, const std::string& autoCompletionValue, bool isPrefixSuggestion, std::size_t newCursorPosition);
		std::string newExpression;
		std::string autoCompletionValue;
		bool isPrefixSuggestion;
		std::size_t newCursorPosition;
	};

	struct PartialExpressionForAutocompletion
	{
		std::string partialExpressionIdentifier;
	};

	static std::vector<AutoCompletionEntry> autoComplete(const std::string& expression, std::size_t cursorPosition, CatRuntimeContext* context);

private:
	static std::vector<IdentifierToken*> getSubExpressionToAutoComplete(const std::vector<ParseToken*>& tokens, int startingTokenIndex);

	static int findStartTokenIndex(int cursorPosition, const std::vector<ParseToken*>& tokens);

	static void addOptionsFromTypeInfo(TypeInfo* typeInfo, std::vector<AutoCompletion::AutoCompletionEntry>& results, 
									   const std::string& lowercasePrefix, const std::string& originalExpression, std::size_t prefixOffset);
	static void addOptionsFromBuiltIn(std::vector<AutoCompletion::AutoCompletionEntry>& results, const std::string& lowercasePrefix, const std::string& originalExpression, std::size_t prefixOffset);
	static void addIfPartialMatch(const std::string& text, std::vector<AutoCompletion::AutoCompletionEntry>& results, const std::string& lowercasePrefix, const std::string& originalExpression, std::size_t prefixOffset);

	static bool isGlobalScopeAutoCompletable(const std::vector<ParseToken*>& tokens, int startingTokenIndex);
};