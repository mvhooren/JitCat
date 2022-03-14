/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat::Tokenizer
{
	class Document;
	class IdentifierToken;
	struct ParseToken;
}
namespace jitcat::Reflection
{
	class TypeInfo;
	struct TypeMemberInfo;
	struct MemberFunctionInfo;
}

#include <memory>
#include <string>
#include <vector>

namespace jitcat
{
	class CatGenericType;
	class CatRuntimeContext;


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
		static std::vector<std::pair<const Tokenizer::ParseToken*, bool>> getSubExpressionToAutoComplete(const std::vector<Tokenizer::ParseToken>& tokens, int startingTokenIndex, std::string& expressionTailEnd);

		static void addOptionsFromGlobalScope(const std::string& prefix, const std::string& expression, const std::string& expressionTailEnd, 
											  std::size_t completionOffset, CatRuntimeContext* context, std::vector<AutoCompletionEntry>& entries);

		static int findStartTokenIndex(const jitcat::Tokenizer::Document& doc, int cursorPosition, const std::vector<Tokenizer::ParseToken>& tokens);

		static void addOptionsFromTypeInfo(Reflection::TypeInfo* typeInfo, std::vector<AutoCompletion::AutoCompletionEntry>& results, 
										   const std::string& lowercasePrefix, const std::string& originalExpression, std::size_t prefixOffset, const std::string& expressionTailEnd);
		static void addOptionsFromBuiltIn(std::vector<AutoCompletion::AutoCompletionEntry>& results, const std::string& lowercasePrefix, const std::string& originalExpression, std::size_t prefixOffset);
		static void addIfPartialMatch(const std::string& text, std::vector<AutoCompletion::AutoCompletionEntry>& results, const std::string& lowercasePrefix, const std::string& originalExpression, std::size_t prefixOffset);

		static bool isGlobalScopeAutoCompletable(const std::vector<Tokenizer::ParseToken>& tokens, int startingTokenIndex);

		static bool autoCompleteOnType(const CatGenericType& type, bool completeOnArrayIndex, const std::string& memberPrefix, 
									   const std::string& expression, std::size_t completionOffset, const std::string& expressionTailEnd, 
									    std::vector<AutoCompletion::AutoCompletionEntry>& results);

		static bool traverseType(const CatGenericType& type, Reflection::TypeMemberInfo*& currentMemberInfo, 
								 Reflection::MemberFunctionInfo*& currentFunctionInfo, const std::string& lowerCaseIdentifier,
								 bool traverseOnArrayIndex);
	};

} //End namespace jitcat