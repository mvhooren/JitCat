/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/AssignableType.h"
#include "jitcat/CatGenericType.h"

#include <any>
#include <memory>

namespace jitcat::AST 
{
	class CatTypedExpression;


	class ASTHelper
	{
	private:
		ASTHelper() = delete;
	public:
		static void updatePointerIfChanged(std::unique_ptr<CatTypedExpression>& uPtr, CatTypedExpression* expression);
		static void doTypeConversion(std::unique_ptr<CatTypedExpression>& uPtr, const CatGenericType& targetType);

		//Source and target must be of the same type and target must be a writable type
		static void doAssignment(std::any& target, std::any& source, const CatGenericType& type, Reflection::AssignableType targetAssignableType);
	};


} //End namespace jitcat::AST