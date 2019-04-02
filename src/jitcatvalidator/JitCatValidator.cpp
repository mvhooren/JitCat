/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcatvalidator/JitCatValidator.h"

#include <jitcat/AutoCompletion.h>
#include <jitcat/CatPrefixOperator.h>
#include <jitcat/CatRuntimeContext.h>
#include <jitcat/CatTypedExpression.h>
#include <jitcat/CustomTypeInfo.h>
#include <jitcat/Document.h>
#include <jitcat/ExpressionErrorManager.h>
#include <jitcat/JitCat.h>
#include <jitcat/SLRParseResult.h>
#include <jitcat/TypeInfo.h>
#include <jitcat/TypeRegistry.h>
#include <jitcat/Tools.h>
#include <memory>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Parser;
using namespace jitcat::Reflection;
using namespace jitcat::Tokenizer;


JITCATVALIDATOR_API int validateExpression(const char* expression, const char* rootScopeTypeNames, ValidationResult* result)
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

	std::vector<std::string> rootScopeTypeList;
	Tools::split(rootScopeTypeNames, " ", rootScopeTypeList); 
	CatRuntimeContext context("", nullptr);
	for (auto& iter : rootScopeTypeList)
	{
		TypeInfo* typeInfo = TypeRegistry::get()->getTypeInfo(iter);
		if (typeInfo == nullptr)
		{
			return -2;
		}
		else
		{
			context.addScope(typeInfo);
		}
	}

	Document document(expression, strlen(expression));
	ExpressionErrorManager errorManager;
	errorManager.setCurrentDocument(&document);
	std::unique_ptr<SLRParseResult> parseResult(JitCat::get()->parseExpression(&document, &context, &errorManager, nullptr));
	CatTypedExpression* typedExpression = nullptr;

	if (parseResult->success)
	{
		typedExpression = parseResult->getNode<CatTypedExpression>();
		
		if (!typedExpression->typeCheck(&context, &errorManager, nullptr))
		{
			parseResult->success = false;
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
			std::string typeName = typedExpression->getType().toString();
			result->typeName = (const char*)malloc(typeName.size() + 1);
			memcpy((void*)result->typeName, typeName.c_str(), typeName.size() + 1);
			result->isConstant = typedExpression->isConst();
			return 1;
		}
	}
	if (!parseResult->success)
	{
		std::string contextName = context.getContextName().c_str();
		std::vector<const ExpressionErrorManager::Error*> errorList;
		errorManager.getAllErrors(errorList);
		std::string errorMessage;
		int errorOffset = 0;
		if (errorList.size() > 0)
		{
			errorMessage = errorList[0]->message;
			errorOffset = (int)document.getOffsetInDocument(errorList[0]->errorLexeme); 
		}
		result->errorMessage = new char[errorMessage.size() + 1];
		memcpy((void*)result->errorMessage, errorMessage.c_str(), errorMessage.size() + 1);
		result->errorOffset = errorOffset;
		parseResult->astRootNode.reset(nullptr);
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


JITCATVALIDATOR_API int codeCompleteExpression(const char* expression, int cursorPosition, const char* rootScopeTypeNames, CodeCompletionSuggestions* results)
{
	if (results == nullptr)
	{
		return -1;
	}

	std::vector<std::string> rootScopeTypeList;
	Tools::split(rootScopeTypeNames, " ", rootScopeTypeList); 
	CatRuntimeContext context("", nullptr);
	for (auto& iter : rootScopeTypeList)
	{
		TypeInfo* typeInfo = TypeRegistry::get()->getTypeInfo(iter);
		if (typeInfo == nullptr)
		{
			return -2;
		}
		else
		{
			context.addScope(typeInfo);
		}
	}

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
