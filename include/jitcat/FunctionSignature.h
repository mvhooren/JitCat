/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"

#include <string>
#include <vector>

namespace jitcat::Reflection
{
	class FunctionSignature
	{
	public:
		FunctionSignature() {}
		virtual const std::string& getLowerCaseFunctionName() const = 0;
		virtual int getNumParameters() const = 0;
		virtual const CatGenericType& getParameterType(int index) const = 0;
	
		bool compare(const FunctionSignature& other) const;
		
	};

	class SearchFunctionSignature: public FunctionSignature
	{
	public:
		SearchFunctionSignature(const std::string& functionName, const std::vector<CatGenericType>& parameterTypes);
		virtual const std::string& getLowerCaseFunctionName() const override final;
		virtual int getNumParameters() const override final;
		virtual const CatGenericType& getParameterType(int index) const override final;

	private:
		std::string lowerCaseName;
		std::vector<CatGenericType> parameterTypes;
	};
}