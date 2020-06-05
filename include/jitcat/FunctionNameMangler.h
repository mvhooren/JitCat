/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <string>
#include <vector>
#include "jitcat/CatGenericType.h"


namespace jitcat
{
	
namespace FunctionNameMangler
{
	std::string getMangledFunctionName(const CatGenericType& returnType, const std::string& functionName, const std::vector<CatGenericType>& parameterTypes, bool isThisCall, const std::string& qualifiedParentClassName);
}

}