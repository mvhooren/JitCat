/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/FunctionSignature.h"
#include "jitcat/Tools.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::Reflection;


bool FunctionSignature::compare(const FunctionSignature& other) const
{
	//First compare lower case names alphabetically.
	int nameComparison = getLowerCaseFunctionName().compare(other.getLowerCaseFunctionName());
	if (nameComparison != 0)
	{
		return false;
	}
	else
	{
		//Same name, check number of parameters
		int numParametersComparison = getNumParameters() - other.getNumParameters();
		if (numParametersComparison != 0)
		{
			return false;
		}
		else
		{
			//Same number of parameters, check parameter types
			for (int i = 0; i < getNumParameters(); i++)
			{
				CatGenericType type = getParameterType(i);
				CatGenericType otherType = other.getParameterType(i);
				if (!type.compare(otherType, false, false))
				{
					return false;
				}
			}
			//Signature is the same
			return true;
		}
	}
}


SearchFunctionSignature::SearchFunctionSignature(const std::string& functionName, const std::vector<CatGenericType>& parameterTypes):
	lowerCaseName(Tools::toLowerCase(functionName)),
	parameterTypes(parameterTypes)
{
}


const std::string& SearchFunctionSignature::getLowerCaseFunctionName() const
{
	return lowerCaseName;
}


int SearchFunctionSignature::getNumParameters() const
{
	return (int)parameterTypes.size();
}


const CatGenericType& SearchFunctionSignature::getParameterType(int index) const
{
	return parameterTypes[index];
}
