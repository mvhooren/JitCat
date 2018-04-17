/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "JitCatValidator.h"

#include <AutoCompletion.h>
#include <CatPrefixOperator.h>
#include <CatRuntimeContext.h>
#include <CatTypedExpression.h>
#include <CustomTypeInfo.h>
#include <Document.h>
#include <ExpressionErrorManager.h>
#include <JitCat.h>
#include <memory>
#include <SLRParseResult.h>
#include <TypeInfo.h>
#include <TypeRegistry.h>


JITCATVALIDATOR_API int validateExpression(const char* expression, const char* globalsTypeName, 
										   const char* localsTypeName, const char* customLocalsTypeName, 
										   const char* customGlobalsTypeName, ValidationResult* result)
{
	if (result == nullptr)
	{
		return -1;
	}
	result->errorMessage = nullptr;
	result->typeName = nullptr;
	result->errorOffset = 0;
	result->isConstant = false;
	result->isLiteral = false;
	result->isValid = false;
	TypeInfo* globalsType = TypeRegistry::get()->getTypeInfo(globalsTypeName);
	TypeInfo* localsType = TypeRegistry::get()->getTypeInfo(localsTypeName);
	TypeInfo* customLocalsType = TypeRegistry::get()->getTypeInfo(customLocalsTypeName);
	TypeInfo* customGlobalsType = TypeRegistry::get()->getTypeInfo(customGlobalsTypeName);
	if ((globalsType == nullptr && strlen(globalsTypeName) != 0)
		|| (localsType == nullptr && strlen(localsTypeName) != 0)
		|| (customLocalsType == nullptr && strlen(customLocalsTypeName) != 0)
		|| (customGlobalsType == nullptr && strlen(customGlobalsTypeName) != 0))
	{
		return -2;
	}
	CatRuntimeContext context(globalsType, localsType, customLocalsType, customGlobalsType, "", false, nullptr);

	Document document(expression, strlen(expression));
	std::unique_ptr<SLRParseResult> parseResult(JitCat::get()->parse(&document, &context));
	CatTypedExpression* typedExpression = nullptr;
	if (parseResult->success)
	{
		typedExpression = static_cast<CatTypedExpression*>(parseResult->astRootNode);
		CatGenericType valueType = typedExpression->typeCheck();
		if (!valueType.isValidType())
		{
			parseResult->success = false;
			parseResult->errorMessage = valueType.getErrorMessage();
		}
		else
		{
			bool isMinusPrefixWithLiteral = false;
			//If the expression is a minus prefix operator combined with a literal, then we need to count the whole expression as a literal.
			if (typedExpression->getNodeType() == CatASTNodeType::PrefixOperator)
			{
				CatPrefixOperator* prefixOp = static_cast<CatPrefixOperator*>(typedExpression);
				if (prefixOp->rhs != nullptr
					&& prefixOp->oper == CatPrefixOperator::Operator::Minus
					&& prefixOp->rhs->getNodeType() == CatASTNodeType::Literal)
				{
					result->isLiteral = true;
				}
			}
			else if (typedExpression->getNodeType() == CatASTNodeType::Literal)
			{
				result->isLiteral = true;
			}
			result->isValid = true;
			std::string typeName = valueType.toString();
			result->typeName = (const char*)malloc(typeName.size() + 1);
			memcpy((void*)result->typeName, typeName.c_str(), typeName.size() + 1);
			result->isConstant = typedExpression->isConst();
			return 1;
		}
	}
	if (!parseResult->success)
	{
		std::string contextName = context.getContextName().c_str();
		result->errorMessage = new char[parseResult->errorMessage.size() + 1];
		memcpy((void*)result->errorMessage, parseResult->errorMessage.c_str(), parseResult->errorMessage.size() + 1);
		result->errorOffset = parseResult->errorPosition;
		parseResult->astRootNode = nullptr;
		return 0;
	}

	return 0;
}


JITCATVALIDATOR_API void destroyValidationResult(ValidationResult* result)
{
	delete result->errorMessage;
	delete result->typeName;
	result->errorMessage = nullptr;
	result->typeName = nullptr;
}


JITCATVALIDATOR_API int codeCompleteExpression(const char* expression, int cursorPosition, const char* globalsTypeName, 
											   const char* localsTypeName, const char* customLocalsTypeName, 
											   const char* customGlobalsTypeName, CodeCompletionSuggestions* results)
{
	if (results == nullptr)
	{
		return -1;
	}
	TypeInfo* globalsType = TypeRegistry::get()->getTypeInfo(globalsTypeName);
	TypeInfo* localsType = TypeRegistry::get()->getTypeInfo(localsTypeName);
	TypeInfo* customLocalsType = TypeRegistry::get()->getTypeInfo(customLocalsTypeName);
	TypeInfo* customGlobalsType = TypeRegistry::get()->getTypeInfo(customGlobalsTypeName);
	if ((globalsType == nullptr && strlen(globalsTypeName) != 0)
		|| (localsType == nullptr && strlen(localsTypeName) != 0)
		|| (customLocalsType == nullptr && strlen(customLocalsTypeName) != 0)
		|| (customGlobalsType == nullptr && strlen(customGlobalsTypeName) != 0))
	{
		return -2;
	}
	CatRuntimeContext context(globalsType, localsType, customLocalsType, customGlobalsType, "Completion", false, nullptr);
	const auto& suggestions = AutoCompletion::autoComplete(expression, cursorPosition, &context);
	if (suggestions.size() > 0)
	{
		std::vector<CodeCompletionSuggestion*> suggestionsToReturn;
		for (auto& suggestion : suggestions)
		{
			CodeCompletionSuggestion* suggestionCopy = new CodeCompletionSuggestion();
			suggestionCopy->autoCompletionValue = new char[suggestion.autoCompletionValue.size() + 1];
			suggestionCopy->newExpression = new char[suggestion.newExpression.size() + 1];
			memcpy((void*)suggestionCopy->autoCompletionValue, suggestion.autoCompletionValue.c_str(), suggestion.autoCompletionValue.size() + 1);
			memcpy((void*)suggestionCopy->newExpression, suggestion.newExpression.c_str(), suggestion.newExpression.size() + 1);
			suggestionCopy->isPrefixSuggestion = suggestion.isPrefixSuggestion;
			suggestionCopy->newCursorPosition = suggestion.newCursorPosition;
			suggestionsToReturn.push_back(suggestionCopy);
		}
		results->suggestions = new CodeCompletionSuggestion*[suggestionsToReturn.size()];
		memcpy(results->suggestions, &suggestionsToReturn[0], suggestionsToReturn.size() * 4);
		results->numSuggestions = suggestionsToReturn.size();
		return 1;
	}
	else
	{
		results->suggestions = nullptr;
		results->numSuggestions = 0;
		return 0;
	}
}


JITCATVALIDATOR_API void destroyCompletionResult(CodeCompletionSuggestions* result)
{
	for (std::size_t i = 0; i < result->numSuggestions; i ++)
	{
		delete[] result->suggestions[i]->autoCompletionValue;
		delete[] result->suggestions[i]->newExpression;
		delete result->suggestions[i];
	}
	delete[] result->suggestions;
	result->suggestions = nullptr;
	result->numSuggestions = 0;
}


JITCATVALIDATOR_API int loadTypeInfo(const char* path)
{
	TypeRegistry::recreate();
	if (TypeRegistry::get()->loadRegistryFromXML(path))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
