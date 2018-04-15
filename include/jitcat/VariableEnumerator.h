/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <string>
#include <vector>


class VariableEnumerator
{
public:
	virtual ~VariableEnumerator() {}

	enum NamespaceType
	{
		NT_CATEGORY,
		NT_OBJECT,
		NT_MAP,
		NT_VECTOR
	};
	virtual void addFunction(const std::string& name, const std::string& prototype) = 0;
	virtual void addVariable(const std::string& name, const std::string& typeName, bool isWritable, bool isConst) = 0;
	virtual void enterNameSpace(const std::string& name, const std::string& typeName, NamespaceType namespaceType) = 0;
	virtual void exitNameSpace() = 0;

	//Used to prevent infinite loops when enumerating types
	std::vector<std::string> loopDetectionTypeStack;
};
