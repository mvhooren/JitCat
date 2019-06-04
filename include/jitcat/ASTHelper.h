/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/Lexeme.h"
#include "jitcat/TypeOwnershipSemantics.h"

#include <any>
#include <memory>

namespace jitcat
{
	class CatRuntimeContext;
	class ExpressionErrorManager;
}

namespace jitcat::AST 
{
	class CatAssignableExpression;
	class CatTypedExpression;


	class ASTHelper
	{
	private:
		ASTHelper() = delete;
	public:
		static void updatePointerIfChanged(std::unique_ptr<CatTypedExpression>& uPtr, CatTypedExpression* expression);
		static void doTypeConversion(std::unique_ptr<CatTypedExpression>& uPtr, const CatGenericType& targetType);

		static std::any doAssignment(CatAssignableExpression* target, CatTypedExpression* source, CatRuntimeContext* context);
		static std::any doGetArgument(CatTypedExpression* argument, const CatGenericType& parameterType, CatRuntimeContext* context);

		//Source and target must be of the same type and target must be a writable type. Source must be a writable type if its ownership semantics are Owned and the target type's ownership semantics is Owned or Shared.
		static std::any doAssignment(std::any& target, const std::any& source, const CatGenericType& targetType, const CatGenericType& sourceType);

		static bool checkAssignment(const CatTypedExpression* lhs, const CatTypedExpression* rhs, ExpressionErrorManager* errorManager, CatRuntimeContext* context, void* errorSource, const Tokenizer::Lexeme& lexeme);
		static bool checkOwnershipSemantics(const CatGenericType& targetType, const CatGenericType& sourceType, ExpressionErrorManager* errorManager, CatRuntimeContext* context, void* errorSource, const Tokenizer::Lexeme& lexeme, const std::string& operation);
	};


} //End namespace jitcat::AST