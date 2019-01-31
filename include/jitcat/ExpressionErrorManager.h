/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/Reflectable.h"

#include <functional>
#include <string>
#include <vector>

namespace jitcat
{

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
			std::string message;
			void* expression;

			static void reflect(Reflection::TypeInfo& typeInfo);
			static const char* getTypeName();

		};

		ExpressionErrorManager(std::function<void(const std::string&)> errorHandler = {});
		~ExpressionErrorManager();

		void clear();
		//Adds an error to the list. The void* serves as a unique id so that error messages disappear once the error is fixed.
		void compiledWithError(const std::string& errorMessage, void* expression);
		void compiledWithoutErrors(void* expression);
		void expressionDeleted(void* expression);

		const std::vector<Error*>& getErrors() const;

		//Whenever errors are added or removed, the error revision is incremented.
		//This is used by the user interface to track changes and update the error list accordingly.
		unsigned int getErrorsRevision() const;

		static void reflect(Reflection::TypeInfo& typeInfo);
		static const char* getTypeName();

	private:
		void deleteErrorsFromExpression(void* expression);

	private:
		std::vector<Error*> errors;
		std::function<void(const std::string&)> errorHandler;
		unsigned int errorsRevision;
	};

} //End namespace jitcat