/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/TypeRegistry.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/TypeInfo.h"
#include "jitcat/XMLHelper.h"


#include <fstream>
#include <iostream>
#include <stddef.h>

using namespace jitcat::Reflection;


TypeRegistry::TypeRegistry()
{
}


TypeRegistry::~TypeRegistry()
{
	for (auto& iter: ownedTypes)
	{
		TypeInfo::destroy(iter);
	}
}


TypeRegistry* TypeRegistry::get()
{
	if (instance == nullptr)
	{
		instance = new TypeRegistry();
	}
	return instance;
}


void TypeRegistry::recreate()
{
	delete instance;
	instance = new TypeRegistry();
}


TypeInfo* TypeRegistry::getTypeInfo(const std::string& typeName)
{
	std::string lowerName = Tools::toLowerCase(typeName);;
	std::map<std::string, TypeInfo*>::iterator iter = types.find(lowerName);
	if (iter != types.end())
	{
		return iter->second;
	}
	else
	{
		return nullptr;
	}
}


TypeInfo* TypeRegistry::getOrCreateTypeInfo(const char* typeName, std::size_t typeSize, TypeCaster* caster, bool allowConstruction, 
											std::function<Reflectable*()>& constructor,
											std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
											std::function<void (Reflectable*)>& destructor,
											std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor)
{
	std::string lowerName = Tools::toLowerCase(typeName);
	std::map<std::string, TypeInfo*>::iterator iter = types.find(lowerName);
	if (iter == types.end())
	{
		TypeInfo* typeInfo = new ReflectedTypeInfo(typeName, typeSize, caster, allowConstruction, constructor, placementConstructor, destructor, placementDestructor);
		types[lowerName] = typeInfo;
		return typeInfo;
	}
	else
	{
		return iter->second;
	}
}


const std::map<std::string, TypeInfo*>& TypeRegistry::getTypes() const
{
	return types;
}


void TypeRegistry::registerType(const char* typeName, TypeInfo* typeInfo)
{
	std::string lowerName = Tools::toLowerCase(typeName);
	std::map<std::string, TypeInfo*>::iterator iter = types.find(lowerName);
	if (iter == types.end())
	{
		types[lowerName] = typeInfo;
	}
	else
	{
		std::cout << "ERROR: duplicate type definition: " << iter->first << ".\n";
	}
}


void TypeRegistry::removeType(const char* typeName)
{
	std::string lowerName = Tools::toLowerCase(typeName);
	std::map<std::string, TypeInfo*>::iterator iter = types.find(lowerName);
	if (iter != types.end())
	{
		//TypeInfo is leaked here, but since removing types is very rare and TypeInfo* are stored everywhere, this accepable.
		//Deleting it would likely cause crashes. Fixing this properly requires a large time investment.
		types.erase(iter);
	}
}


void TypeRegistry::renameType(const std::string& oldName, const char* newTypeName)
{
	std::string oldLower = Tools::toLowerCase(oldName);
	std::string newLower = Tools::toLowerCase(newTypeName);
	std::map<std::string, TypeInfo*>::iterator iter = types.find(oldLower);
	if (iter != types.end() && types.find(newLower) == types.end())
	{
		TypeInfo* oldTypeInfo = iter->second;
		types.erase(iter);
		oldTypeInfo->setTypeName(newTypeName);
		types[newLower] = oldTypeInfo;
	}
}

enum class XMLReadState
{
	ReadingRegistry,
	ReadingType,
	ReadingMembers,
	ReadingMemberFunctions,
};


bool TypeRegistry::loadRegistryFromXML(const std::string& filepath)
{
	std::ifstream xmlFile;
	xmlFile.open(filepath);
	if (xmlFile.is_open())
	{
		XMLLineType tagType;
		std::string contents;
		std::string tagName = XMLHelper::readXMLLine(xmlFile, tagType, contents);

		TypeInfo* currentTypeInfo = nullptr;

		std::map<std::string, TypeInfo*> typeInfos;
		if (tagType == XMLLineType::OpenTag && tagName == "TypeRegistry")
		{
			XMLReadState readState = XMLReadState::ReadingRegistry;
			while (true)
			{
				tagName = XMLHelper::readXMLLine(xmlFile, tagType, contents);
				switch (readState)
				{
					case XMLReadState::ReadingRegistry:
					{
						if (tagType == XMLLineType::CloseTag && tagName == "TypeRegistry")
						{
							//We are done
							return true;
						}
						else if (tagType == XMLLineType::OpenTag && tagName == "Type")
						{
							readState = XMLReadState::ReadingType;
						}
						else
						{
							return false;
						}
					} break;
					case XMLReadState::ReadingType:
					{

						if (tagType == XMLLineType::OpenCloseWithContent && tagName == "Name" && contents != "" && currentTypeInfo == nullptr)
						{
							currentTypeInfo = XMLHelper::findOrCreateTypeInfo(contents, typeInfos);
						}
						else if (currentTypeInfo != nullptr && tagType == XMLLineType::OpenTag)
						{
							if (tagName == "Members")
							{
								readState = XMLReadState::ReadingMembers;
							}
							else if (tagName == "MemberFunctions")
							{
								readState = XMLReadState::ReadingMemberFunctions;
							}
							else
							{
								return false;
							}
						}
						else if (currentTypeInfo != nullptr && tagType == XMLLineType::CloseTag && tagName == "Type")
						{
							if (types.find(currentTypeInfo->getTypeName()) == types.end())
							{
								types[Tools::toLowerCase(currentTypeInfo->getTypeName())] = currentTypeInfo;
								currentTypeInfo = nullptr;
								readState = XMLReadState::ReadingRegistry;
							}
						}
						else
						{
							return false;
						}
					} break;
					case XMLReadState::ReadingMembers:
					{
						if (tagType == XMLLineType::CloseTag && tagName == "Members")
						{
							readState = XMLReadState::ReadingType;
						}
						else if (tagType == XMLLineType::OpenTag && tagName == "Member")
						{
							if (!XMLHelper::readMember(xmlFile, currentTypeInfo, typeInfos))
							{
								return false;
							}
						}
						else
						{
							return false;
						}
					} break;
					case XMLReadState::ReadingMemberFunctions:
					{
						if (tagType == XMLLineType::CloseTag && tagName == "MemberFunctions")
						{
							readState = XMLReadState::ReadingType;
						}
						else if (tagType == XMLLineType::OpenTag && tagName == "MemberFunction")
						{
							if (!XMLHelper::readMemberFunction(xmlFile, currentTypeInfo, typeInfos))
							{
								return false;
							}
						}
						else
						{
							return false;
						}
					} break;
				}
			}
		}
	}
	return false;
}


void TypeRegistry::exportRegistyToXML(const std::string& filepath)
{
	std::ofstream xmlFile;
	xmlFile.open(filepath);
	xmlFile << "<TypeRegistry>\n";
	for (auto& iter : types)
	{
		xmlFile << "\t<Type>\n";
		/*if (iter.second->isCustomType())
		{
			xmlFile << "\t\t<custom/>\n";
		}*/
		xmlFile << "\t\t<Name>" << iter.second->getTypeName() << "</Name>\n";
		if (iter.second->getMembers().size() > 0)
		{
			xmlFile << "\t\t<Members>\n";
			for (auto& member : iter.second->getMembers())
			{
				xmlFile << "\t\t\t<Member>\n";
				xmlFile << "\t\t\t\t<Name>" << member.second->memberName << "</Name>\n";

				member.second->catType.writeToXML(xmlFile, "\t\t\t\t");
			
				xmlFile << "\t\t\t</Member>\n";
			}
			xmlFile << "\t\t</Members>\n";
		}
		if (iter.second->getMemberFunctions().size() > 0)
		{
			xmlFile << "\t\t<MemberFunctions>\n";
			for (auto& member : iter.second->getMemberFunctions())
			{
				xmlFile << "\t\t\t<MemberFunction>\n";
				xmlFile << "\t\t\t\t<Name>" << member.second->memberFunctionName << "</Name>\n";		
				xmlFile << "\t\t\t\t<ReturnType>\n";
				member.second->returnType.writeToXML(xmlFile, "\t\t\t\t\t");
				xmlFile << "\t\t\t\t</ReturnType>\n";
				xmlFile << "\t\t\t\t<Arguments>\n";
				for (auto& argument : member.second->argumentTypes)
				{
					xmlFile << "\t\t\t\t\t<Argument>\n";
					argument.writeToXML(xmlFile, "\t\t\t\t\t\t");
					xmlFile << "\t\t\t\t\t</Argument>\n";
				}
				xmlFile << "\t\t\t\t</Arguments>\n";
				xmlFile << "\t\t\t</MemberFunction>\n";
			}
			xmlFile << "\t\t</MemberFunctions>\n";
		}
		xmlFile << "\t</Type>\n";
	}
	xmlFile << "</TypeRegistry>\n";
	xmlFile.close();
}


ReflectedTypeInfo* TypeRegistry::createTypeInfo(const char* typeName, std::size_t typeSize, TypeCaster* typeCaster, bool allowConstruction, 
												std::function<Reflectable*()>& constructor,
												std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
												std::function<void (Reflectable*)>& destructor,
												std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor)
{
	return new ReflectedTypeInfo(typeName, typeSize, typeCaster, allowConstruction, constructor, placementConstructor, destructor, placementDestructor);
}


TypeInfo* jitcat::Reflection::TypeRegistry::castToTypeInfo(ReflectedTypeInfo* reflectedTypeInfo)
{
	return static_cast<TypeInfo*>(reflectedTypeInfo);
}


TypeRegistry* TypeRegistry::instance = nullptr;
