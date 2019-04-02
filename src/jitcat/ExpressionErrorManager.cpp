/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/Document.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;
using namespace jitcat::Tokenizer;


ExpressionErrorManager::ExpressionErrorManager(std::function<void(const std::string&, int, int, int)> errorHandler):
	errorsRevision(0),
	errorHandler(errorHandler),
	currentDocument(nullptr)
{
}


ExpressionErrorManager::~ExpressionErrorManager()
{
	clear();
}


void jitcat::ExpressionErrorManager::setCurrentDocument(Tokenizer::Document* document)
{
	currentDocument = document;
}


void ExpressionErrorManager::clear()
{
	errors.clear();
	errorsRevision++;
}


void ExpressionErrorManager::compiledWithError(const std::string& errorMessage, void* errorSource, const std::string& contextName, const jitcat::Tokenizer::Lexeme& errorLexeme)
{
	deleteErrorsFromSource(errorSource);

	Error* error = new Error();
	error->message = errorMessage;
	error->contextName = contextName;
	error->errorLexeme = errorLexeme;
	if (currentDocument != nullptr)
	{
		auto [line, column, length] = currentDocument->getLineColumnAndLength(errorLexeme);
		error->errorLine = line;
		error->errorColumn = column;
		error->errorLength = length;
	}
	else
	{
		error->errorLine = 0;
		error->errorColumn = 0;
		error->errorLength = 0;
	}
	error->errorSource = errorSource;
	errors.emplace(errorSource, error);
	errorsRevision++;

	if (errorHandler)
	{
		errorHandler(Tools::append(contextName, "\n", errorMessage), error->errorLine, error->errorColumn, error->errorLength);
	}
}


void ExpressionErrorManager::compiledWithoutErrors(void* errorSource)
{
	deleteErrorsFromSource(errorSource);
}


void ExpressionErrorManager::errorSourceDeleted(void* errorSource)
{
	deleteErrorsFromSource(errorSource);
}


void jitcat::ExpressionErrorManager::getAllErrors(std::vector<const Error*>& allErrors) const
{
	for (auto& iter : errors)
	{
		allErrors.push_back(iter.second.get());
	}
}


unsigned int ExpressionErrorManager::getErrorsRevision() const
{
	return errorsRevision;
}


void ExpressionErrorManager::reflect(TypeInfo& typeInfo)
{
	//typeInfo.addMember("errors", &ExpressionErrorManager::errors);
}


const char* ExpressionErrorManager::getTypeName()
{
	return "ExpressionErrorManager";
}


void ExpressionErrorManager::deleteErrorsFromSource(void* errorSource)
{
	errors.erase(errorSource);
}


void ExpressionErrorManager::Error::reflect(TypeInfo& typeInfo)
{
	typeInfo.addMember("message", &ExpressionErrorManager::Error::message);
}


const char* ExpressionErrorManager::Error::getTypeName()
{
	return "ExpressionError";
}
