/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

class TypeInfo;
#include "CatGenericType.h"

#include <fstream>
#include <map>
#include <vector>


enum class XMLLineType
{
	OpenTag,
	CloseTag,
	SelfClosingTag,
	OpenCloseWithContent,
	Error
};

class XMLHelper
{
public:
	static std::string readXMLLine(std::ifstream& xmlFile, XMLLineType& tagType, std::string& contents);
	static bool readMember(std::ifstream& xmlFile, TypeInfo* currentType, std::map<std::string, TypeInfo*>& typeInfos);
	static bool readMemberFunction(std::ifstream& xmlFile, TypeInfo* currentType, std::map<std::string, TypeInfo*>& typeInfos);
	static TypeInfo* findOrCreateTypeInfo(const std::string& typeName, std::map<std::string, TypeInfo*>& typeInfos);

private:
	static std::vector<const char*> staticNames;
};