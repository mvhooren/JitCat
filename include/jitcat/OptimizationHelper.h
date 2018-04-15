/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class CatTypedExpression;

#include <memory>


class OptimizationHelper
{
private:
	OptimizationHelper() = delete;
public:
	static void updatePointerIfChanged(std::unique_ptr<CatTypedExpression>& uPtr, CatTypedExpression* expression);
};