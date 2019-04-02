/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Lexeme.h"
#include "jitcat/Reflectable.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace jitcat
{
	namespace Tokenizer
	{
		class Document;
	};
	class ExpressionErrorManager: public Reflection::Reflectable
	{
	private:
		ExpressionErrorManager(const ExpressionErrorManager&) = delete;
	public:
		struct Error: public Reflection::Reflectable
		{
		private:
			Error(const Error&) = delete;
		public:
			Error() {}
			std::string contextName;
			jitcat::Tokenizer::Lexeme errorLexeme;
			int errorLine;
			int errorColumn;
			int errorLength;
			std::string message;
			void* errorSource;

			static void reflect(Reflection::TypeInfo& typeInfo);
			static const char* getTypeName();

		};

		ExpressionErrorManager(std::function<void(const std::string&, int, int, int)> errorHandler = {});
		~ExpressionErrorManager();

		//The error manager needs to know about the current document if it wants to properly translate lexemes to line and column numbers.
		//errorLine, errorColumn and errorLength will be set to 0 on all errors if a document is not set.
		void setCurrentDocument(Tokenizer::Document* document);

		void clear();
		//Adds an error to the list. The void* serves as a unique id so that error messages disappear once the error is fixed.
		void compiledWithError(const std::string& errorMessage, void* errorSource, const std::string& contextName, const jitcat::Tokenizer::Lexeme& errorLexeme);
		void compiledWithoutErrors(void* errorSource);
		void errorSourceDeleted(void* errorSource);

		void getAllErrors(std::vector<const Error*>& errors) const;

		//Whenever errors are added or removed, the error revision is incremented.
		//This is used by the user interface to track changes and update the error list accordingly.
		unsigned int getErrorsRevision() const;

		static void reflect(Reflection::TypeInfo& typeInfo);
		static const char* getTypeName();

	private:
		void deleteErrorsFromSource(void* errorSource);

	private:
		Tokenizer::Document* currentDocument;
		std::multimap<void*, std::unique_ptr<Error>> errors;
		std::function<void(const std::string&, int, int, int)> errorHandler;
		unsigned int errorsRevision;
	};

} //End namespace jitcat