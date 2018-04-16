/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "XMLHelper.h"
#include "TypeInfo.h"
#include "MemberInfo.h"
#include "MemberFunctionInfo.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


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
	SpecificMemberType currentSpecificMemberType = SpecificMemberType::None;
	CatType currentMemberBasicType = CatType::Unknown;
	ContainerType currentMemberContainerType = ContainerType::None;
	std::string currentMemberContainerItemTypename = "";
	std::string currentMemberObjectTypeName = "";
	std::string currentMemberName = "";
	bool currentMemberIsConst = false;
	bool currentMemberIsWritable = false;
	while (true)
	{
		XMLLineType tagType;
		std::string contents;
		std::string tagName = XMLHelper::readXMLLine(xmlFile, tagType, contents);
		if (tagType == XMLLineType::OpenCloseWithContent)
		{
			if (tagName == "Name")
			{
				currentMemberName = contents;
			}
			else if (tagName == "Type")
			{
				currentSpecificMemberType = toSpecificMemberType(contents.c_str());
			}
			else if (tagName == "BasicType")
			{
				currentMemberBasicType = toCatType(contents.c_str());
			}
			else if (tagName == "ContainerType")
			{
				currentMemberContainerType = toContainerType(contents.c_str());
			}
			else if (tagName == "ContainerItemTypeName")
			{
				currentMemberContainerItemTypename = contents;
			}
			else if (tagName == "ObjectTypeName")
			{
				currentMemberObjectTypeName = contents;
			}
			else
			{
				return false;
			}
		}
		else if (tagType == XMLLineType::SelfClosingTag)
		{
			if (tagName == "const")
			{
				currentMemberIsConst = true;
			}
			else if (tagName == "writable")
			{
				currentMemberIsWritable = true;
			}
			else
			{
				return false;
			}
		}
		else if (tagType == XMLLineType::CloseTag && tagName == "Member")
		{
			if (currentMemberName != "")
			{
				switch (currentSpecificMemberType)
				{
					case SpecificMemberType::CatType:
						if (currentMemberBasicType != CatType::Unknown)
						{
							currentTypeInfo->addDeserializedMember(new TypeMemberInfo(currentMemberName, currentMemberBasicType, currentMemberIsConst, currentMemberIsWritable));
						}
						else
						{
							return false;
						}
						break;
					case SpecificMemberType::ContainerType:
						if (currentMemberContainerType != ContainerType::None
							&& currentMemberContainerItemTypename != "")
						{
							TypeInfo* itemType = findOrCreateTypeInfo(currentMemberContainerItemTypename, typeInfos);
							currentTypeInfo->addDeserializedMember(new TypeMemberInfo(currentMemberName, currentMemberContainerType, itemType, currentMemberIsConst, currentMemberIsWritable));
						}
						else
						{
							return false;
						}
						break;
					case SpecificMemberType::NestedType:
						if (currentMemberObjectTypeName != "")
						{
							TypeInfo* objectType = findOrCreateTypeInfo(currentMemberObjectTypeName, typeInfos);
							currentTypeInfo->addDeserializedMember(new TypeMemberInfo(currentMemberName, objectType, currentMemberIsConst, currentMemberIsWritable));
						}
						else
						{
							return false;
						}
						break;
					default:
						return false;
				}
				return true;
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
				CatGenericType returnType;
				if (functionInfo == nullptr && readGenericType(xmlFile, "ReturnType", returnType, typeInfos))
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
			else if (tagName == "Argument")
			{
				CatGenericType argumentType;
				if (functionInfo != nullptr && readGenericType(xmlFile, "Argument", argumentType, typeInfos))
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


TypeInfo* XMLHelper::findOrCreateTypeInfo(const std::string & typeName, std::map<std::string, TypeInfo*>& typeInfos)
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
		TypeInfo* newInfo = new TypeInfo(name);
		typeInfos[name] = newInfo;
		return newInfo;
	}
}


bool XMLHelper::readGenericType(std::ifstream& xmlFile, const std::string& closingTag, CatGenericType& type, std::map<std::string, TypeInfo*>& typeInfos)
{
	SpecificMemberType specificType = SpecificMemberType::None;
	CatType basicType = CatType::Unknown;
	std::string objectTypeName = "";
	std::string containerItemTypeName = "";
	ContainerType containerType = ContainerType::None;
	while (true)
	{
		XMLLineType tagType;
		std::string contents;
		std::string tagName = XMLHelper::readXMLLine(xmlFile, tagType, contents);
		if (tagType == XMLLineType::OpenCloseWithContent)
		{
			if (tagName == "Type")
			{
				specificType = toSpecificMemberType(contents.c_str());
			}
			else if (tagName == "BasicType")
			{
				basicType = toCatType(contents.c_str());
			}
			else if (tagName == "ObjectTypeName")
			{
				objectTypeName = contents;
			}
			else if (tagName == "ContainerType")
			{
				containerType = toContainerType(contents.c_str());
			}
			else if (tagName == "ContainerItemTypeName")
			{
				containerItemTypeName = contents;
			}
			else
			{
				return false;
			}
		}
		else if (tagType == XMLLineType::CloseTag && tagName == closingTag)
		{
			switch (specificType)
			{
				case SpecificMemberType::CatType:
					if (basicType != CatType::Unknown)
					{
						type = CatGenericType(basicType);
						return true;
					}
					else
					{
						return false;
					}
					break;
				case SpecificMemberType::NestedType:
					if (objectTypeName != "")
					{
						TypeInfo* objectType = findOrCreateTypeInfo(objectTypeName, typeInfos);
						type = CatGenericType(objectType);
						return true;
					}
					else
					{
						return false;
					}
					break;
				case SpecificMemberType::ContainerType:
					if (containerType != ContainerType::None
						&& containerItemTypeName != "")
					{
						TypeInfo* itemType = findOrCreateTypeInfo(containerItemTypeName, typeInfos);
						type = CatGenericType(containerType, itemType);
						return true;
					}
					else
					{
						return false;
					}
					break;
				case SpecificMemberType::None:
					type = CatGenericType();
					return true;
				default: 
					return false;
			}
		}
		else
		{
			return false;
		}
	}
}


std::vector<const char*> XMLHelper::staticNames = std::vector<const char*>();