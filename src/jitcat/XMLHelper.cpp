/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/XMLHelper.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/MemberInfo.h"
#include "jitcat/MemberFunctionInfo.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


using namespace jitcat::Reflection;

std::string XMLHelper::readXMLLine(std::ifstream& xmlFile, XMLLineType& tagType, std::string& contents)
{
	std::string line;
	std::getline(xmlFile >> std::ws, line);
	if (line.size() > 2
		&& line[0] == '<')
	{
		std::string tagName;
		std::stringstream stream;
		bool closing = false;
		bool previousClosing = false;
		bool selfClosing = false;
		int offset = 1;
		bool done = false;
		for (offset = 1; offset < (int)line.size(); offset++)
		{
			switch (line[offset])
			{
				case '/':	closing = true;	previousClosing = true;	break;
				case '>':	tagName = stream.str(); selfClosing = previousClosing; done = true; offset++; break;
				default:	previousClosing = false; stream << line[offset];	break;
			}
			if (done)
			{
				break;
			}
		}
		if (!closing)
		{
			if (offset < (int)line.size() - 2)
			{
				tagType = XMLLineType::OpenCloseWithContent;
				std::stringstream contentStream;
				for (; offset < (int)line.size(); offset++)
				{
					if (line[offset] != '<')
					{
						contentStream << line[offset];
					}
					else
					{
						contents = contentStream.str();
						break;
					}
				}
			}
			else
			{
				tagType = XMLLineType::OpenTag;
			}
		}
		else if (selfClosing)
		{
			tagType = XMLLineType::SelfClosingTag;
		}
		else
		{
			tagType = XMLLineType::CloseTag;
		}
		return tagName;
	}
	else
	{
		tagType = XMLLineType::Error;
		return "";
	}
}


bool XMLHelper::readMember(std::ifstream& xmlFile, TypeInfo* currentTypeInfo, std::map<std::string, TypeInfo*>& typeInfos)
{
	std::string currentMemberName = "";
	CatGenericType currentMemberType;
	XMLLineType tagType;
	std::string contents;
	std::string tagName = XMLHelper::readXMLLine(xmlFile, tagType, contents);
	if (tagType == XMLLineType::OpenCloseWithContent)
	{
		if (tagName == "Name")
		{
			currentMemberName = contents;
			currentMemberType = CatGenericType::readFromXML(xmlFile, "Member", typeInfos);
			if (currentMemberName != "" && currentMemberType.isValidType())
			{
				currentTypeInfo->addDeserializedMember(new TypeMemberInfo(currentMemberName, currentMemberType));
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}


bool XMLHelper::readMemberFunction(std::ifstream& xmlFile, TypeInfo* currentType, std::map<std::string, TypeInfo*>& typeInfos)
{
	MemberFunctionInfo* functionInfo = nullptr;
	std::string functionName = "";
	while (true)
	{
		XMLLineType tagType;
		std::string contents;
		std::string tagName = XMLHelper::readXMLLine(xmlFile, tagType, contents);
		if (tagType == XMLLineType::CloseTag && tagName == "MemberFunction")
		{
			if (functionInfo != nullptr)
			{
				currentType->addDeserializedMemberFunction(functionInfo);
				return true;
			}
			else
			{
				return false;
			}
		}
		else if (tagType == XMLLineType::OpenCloseWithContent)
		{
			if (tagName == "Name")
			{
				functionName = contents;
			}
			else
			{
				return false;
			}
		}
		else if (tagType == XMLLineType::OpenTag)
		{
			if (tagName == "ReturnType")
			{
				if (functionInfo != nullptr)
				{
					return false;
				}
				CatGenericType returnType = CatGenericType::readFromXML(xmlFile, "ReturnType", typeInfos);
				if (returnType.isValidType())
				{
					functionInfo = new MemberFunctionInfo(functionName, returnType);
				}
				else
				{
					return false;
				}
			}
			else if (tagName == "Arguments")
			{
				continue;
			}
			else if (tagName == "Argument" && functionInfo != nullptr)
			{
				CatGenericType argumentType = CatGenericType::readFromXML(xmlFile, "Argument", typeInfos);
				if (argumentType.isValidType())
				{
					functionInfo->argumentTypes.push_back(argumentType);
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else if (tagType == XMLLineType::CloseTag && tagName == "Arguments")
		{
			continue;
		}
		else
		{
			return false;
		}
	}
}


TypeInfo* XMLHelper::findOrCreateTypeInfo(const std::string& typeName, std::map<std::string, TypeInfo*>& typeInfos)
{
	if (typeInfos.find(typeName) != typeInfos.end())
	{
		return typeInfos[typeName];
	}
	else
	{
		char* name = (char*)malloc(typeName.size() + 1);
		memcpy(name, typeName.c_str(), typeName.size() + 1);
		staticNames.push_back(name);
		TypeInfo* newInfo = new TypeInfo(name, 0, nullptr);
		typeInfos[name] = newInfo;
		return newInfo;
	}
}


std::vector<const char*> XMLHelper::staticNames = std::vector<const char*>();