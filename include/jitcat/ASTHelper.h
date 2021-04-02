/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/FunctionSignature.h"
#include "jitcat/IndirectionConversionMode.h"
#include "jitcat/Lexeme.h"
#include "jitcat/TypeOwnershipSemantics.h"

#include <any>
#include <memory>
#include <vector>


namespace jitcat
{
	class CatRuntimeContext;
	class ExpressionErrorManager;
	namespace Reflection
	{
		struct MemberFunctionInfo;
		class TypeInfo;
	}
}

namespace jitcat::AST 
{
	class CatAssignableExpression;
	class CatIdentifier;
	class CatTypedExpression;
	class CatScopeBlock;
	class CatStatement;

	
	class ASTHelper
	{
	private:
		ASTHelper() = delete;
	public:
		static void updatePointerIfChanged(std::unique_ptr<CatScopeBlock>& uPtr, CatStatement* statement);
		static void updatePointerIfChanged(std::unique_ptr<CatStatement>& uPtr, CatStatement* statement);
		static void updatePointerIfChanged(std::unique_ptr<CatTypedExpression>& uPtr, CatStatement* expression);
		static void updatePointerIfChanged(std::unique_ptr<CatAssignableExpression>& uPtr, CatStatement* expression);
		static void updatePointerIfChanged(std::unique_ptr<CatIdentifier>& uPtr, CatStatement* expression);
		//Returns true if a conversion was inserted
		static bool doTypeConversion(std::unique_ptr<CatTypedExpression>& uPtr, const CatGenericType& targetType);
		//This inserts a CatIndirectionConversion before the CatTypedExpression contained in uPtr.
		static bool doIndirectionConversion(std::unique_ptr<CatTypedExpression>& uPtr, const CatGenericType& expectedType, bool allowAddressOf, IndirectionConversionMode& conversionMode);

		static std::any doAssignment(CatAssignableExpression* target, CatTypedExpression* source, CatRuntimeContext* context);
		static std::any doGetArgument(CatTypedExpression* argument, const CatGenericType& parameterType, CatRuntimeContext* context);

		//Source and target must be of the same type and target must be a writable type. Source must be a writable type if its ownership semantics are Owned and the target type's ownership semantics is Owned or Shared.
		static std::any doAssignment(std::any& target, std::any& source, const CatGenericType& targetType, const CatGenericType& sourceType);

		//Searches type for a function that matches the provided name and argument types. 
		//Generates an error if the function was not found, or if the argumentTypes did not match the expected types.
		//Returns the MemberFunctionInfo if a function was found, nullptr otherwise.
		static Reflection::MemberFunctionInfo* memberFunctionSearch(const std::string& functionName, const std::vector<CatGenericType>& argumentTypes, 
																	Reflection::TypeInfo* type, ExpressionErrorManager* errorManager, CatRuntimeContext* context, 
																	void* errorSource, const Tokenizer::Lexeme& lexeme);

		static std::string getTypeListString(const std::vector<CatGenericType>& types);

		static bool checkAssignment(const CatTypedExpression* lhs, const CatTypedExpression* rhs, ExpressionErrorManager* errorManager, CatRuntimeContext* context, void* errorSource, const Tokenizer::Lexeme& lexeme);
		static bool checkOwnershipSemantics(const CatGenericType& targetType, const CatGenericType& sourceType, ExpressionErrorManager* errorManager, CatRuntimeContext* context, void* errorSource, const Tokenizer::Lexeme& lexeme, const std::string& operation);
	};


} //End namespace jitcat::AST