/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatTypedExpression;

#include "AssignableType.h"
#include "CatGenericType.h"

#include <any>
#include <memory>


class ASTHelper
{
private:
	ASTHelper() = delete;
public:
	static void updatePointerIfChanged(std::unique_ptr<CatTypedExpression>& uPtr, CatTypedExpression* expression);
	static void doTypeConversion(std::unique_ptr<CatTypedExpression>& uPtr, const CatGenericType& targetType);
	//Source and target must be of the same type and target must be a writable type
	static void doAssignment(std::any& target, std::any& source, const CatGenericType& type, AssignableType targetAssignableType);
};