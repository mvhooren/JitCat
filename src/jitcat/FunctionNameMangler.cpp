/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/FunctionNameMangler.h"
#include "jitcat/Tools.h"


using namespace jitcat;
using namespace jitcat::FunctionNameMangler;

std::string FunctionNameMangler::getMangledFunctionName(const CatGenericType& returnType_, const std::string& functionName, const std::vector<CatGenericType>& parameterTypes, 
														bool isThisCall, const std::string& qualifiedParentClassName, bool sRetBeforeThis)
{
	const CatGenericType* returnType = &returnType_;
	std::vector<std::string> parameterNames;
	if (returnType_.isReflectableObjectType())
	{
		returnType = &CatGenericType::voidType;
		
		if (sRetBeforeThis)
		{
			parameterNames.push_back(Tools::append("__sret ", returnType_.toString()));
		}
		if (isThisCall)
		{
			parameterNames.push_back(Tools::append("__this ", qualifiedParentClassName));
		}
		if (!sRetBeforeThis)
		{
			parameterNames.push_back(Tools::append("__sret ", returnType_.toString()));
		}
	}
	else
	{
		if (isThisCall)
		{
			parameterNames.push_back(Tools::append("__this ", qualifiedParentClassName));
		}
	}
	
	for (auto& iter: parameterTypes)
	{
		parameterNames.push_back(iter.toString());
	}

	std::ostringstream stream;
	std::string qualifiedParent;
	if (qualifiedParentClassName != "")
	{
		qualifiedParent = qualifiedParentClassName + "::";
	}
	stream << returnType->toString() << " " << qualifiedParent << functionName << "(";
	for (std::size_t i = 0; i < parameterNames.size(); ++i)
	{
		if (i != 0)
		{
			stream << ", ";
		}
		stream << parameterNames[i];
	}
	stream << ")";
	return stream.str();
}

