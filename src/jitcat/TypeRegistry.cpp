/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/TypeRegistry.h"
#include "jitcat/ReflectedEnumTypeInfo.h"
#include "jitcat/ReflectedTypeInfo.h"
#include "jitcat/StaticMemberInfo.h"
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
	ReadingStaticMembers,
	ReadingMemberFunctions,
	ReadingStaticMemberFunctions
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
		auto iter = types.find("string");
		if (iter != types.end())
		{
			typeInfos["string"] = iter->second;
		}
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
							currentTypeInfo = XMLHelper::findOrCreateTypeInfo(Tools::fromXMLCompatible(contents), typeInfos);
						}
						else if (currentTypeInfo != nullptr && tagType == XMLLineType::OpenTag)
						{
							if (tagName == "Members")
							{
								readState = XMLReadState::ReadingMembers;
							}
							else if (tagName == "StaticMembers")
							{
								readState = XMLReadState::ReadingStaticMembers;
							}
							else if (tagName == "MemberFunctions")
							{
								readState = XMLReadState::ReadingMemberFunctions;
							}
							else if (tagName == "StaticMemberFunctions")
							{
								readState = XMLReadState::ReadingStaticMemberFunctions;
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
					case XMLReadState::ReadingStaticMembers:
					{
						if (tagType == XMLLineType::CloseTag && tagName == "StaticMembers")
						{
							readState = XMLReadState::ReadingType;
						}
						else if (tagType == XMLLineType::OpenTag && tagName == "StaticMember")
						{
							if (!XMLHelper::readStaticMember(xmlFile, currentTypeInfo, typeInfos, currentTypeInfo->getTypeName()))
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
							if (!XMLHelper::readMemberFunction(xmlFile, currentTypeInfo, typeInfos, false))
							{
								return false;
							}
						}
						else
						{
							return false;
						}
					} break;
					case XMLReadState::ReadingStaticMemberFunctions:
					{
						if (tagType == XMLLineType::CloseTag && tagName == "StaticMemberFunctions")
						{
							readState = XMLReadState::ReadingType;
						}
						else if (tagType == XMLLineType::OpenTag && tagName == "StaticMemberFunction")
						{
							if (!XMLHelper::readMemberFunction(xmlFile, currentTypeInfo, typeInfos, true))
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
		if (iter.first == "string") continue;

		xmlFile << "\t<Type>\n";
		xmlFile << "\t\t<Name>" << Tools::toXMLCompatible(iter.second->getTypeName()) << "</Name>\n";
		if (iter.second->getMembers().size() > 0)
		{
			xmlFile << "\t\t<Members>\n";
			for (auto& member : iter.second->getMembers())
			{
				xmlFile << "\t\t\t<Member>\n";
				xmlFile << "\t\t\t\t<Name>" << member.second->getMemberName() << "</Name>\n";

				member.second->getType().writeToXML(xmlFile, "\t\t\t\t");
			
				xmlFile << "\t\t\t</Member>\n";
			}
			xmlFile << "\t\t</Members>\n";
		}
		if (iter.second->getStaticMembers().size() > 0)
		{
			xmlFile << "\t\t<StaticMembers>\n";
			for (auto& member : iter.second->getStaticMembers())
			{
				xmlFile << "\t\t\t<StaticMember>\n";
				xmlFile << "\t\t\t\t<Name>" << member.second->memberName << "</Name>\n";

				member.second->catType.writeToXML(xmlFile, "\t\t\t\t");
			
				xmlFile << "\t\t\t</StaticMember>\n";
			}
			xmlFile << "\t\t</StaticMembers>\n";
		}
		if (iter.second->getMemberFunctions().size() > 0)
		{
			xmlFile << "\t\t<MemberFunctions>\n";
			for (auto& member : iter.second->getMemberFunctions())
			{
				xmlFile << "\t\t\t<MemberFunction>\n";
				xmlFile << "\t\t\t\t<Name>" << Tools::toXMLCompatible(member.second->getMemberFunctionName()) << "</Name>\n";		
				xmlFile << "\t\t\t\t<ReturnType>\n";
				member.second->getReturnType().writeToXML(xmlFile, "\t\t\t\t\t");
				xmlFile << "\t\t\t\t</ReturnType>\n";
				xmlFile << "\t\t\t\t<Arguments>\n";
				for (auto& argument : member.second->getArgumentTypes())
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
		if (iter.second->getStaticMemberFunctions().size() > 0)
		{
			xmlFile << "\t\t<StaticMemberFunctions>\n";
			for (auto& member : iter.second->getStaticMemberFunctions())
			{
				xmlFile << "\t\t\t<StaticMemberFunction>\n";
				xmlFile << "\t\t\t\t<Name>" << Tools::toXMLCompatible(member.second->getNormalFunctionName()) << "</Name>\n";		
				xmlFile << "\t\t\t\t<ReturnType>\n";
				member.second->getReturnType().writeToXML(xmlFile, "\t\t\t\t\t");
				xmlFile << "\t\t\t\t</ReturnType>\n";
				xmlFile << "\t\t\t\t<Arguments>\n";
				for (auto& argument : member.second->getArgumentTypes())
				{
					xmlFile << "\t\t\t\t\t<Argument>\n";
					argument.writeToXML(xmlFile, "\t\t\t\t\t\t");
					xmlFile << "\t\t\t\t\t</Argument>\n";
				}
				xmlFile << "\t\t\t\t</Arguments>\n";
				xmlFile << "\t\t\t</StaticMemberFunction>\n";
			}
			xmlFile << "\t\t</StaticMemberFunctions>\n";
		}
		xmlFile << "\t</Type>\n";
	}
	xmlFile << "</TypeRegistry>\n";
	xmlFile.close();
}


std::unique_ptr<TypeInfo, TypeInfoDeleter> TypeRegistry::createTypeInfo(const char* typeName, std::size_t typeSize, std::unique_ptr<TypeCaster> typeCaster, bool allowConstruction,
												bool allowCopyConstruction, bool allowMoveConstruction, bool triviallyCopyable, bool triviallyConstructable,
												std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
												std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& copyConstructor,
												std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& moveConstructor,
												std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor)
{
	std::unique_ptr<TypeInfo, TypeInfoDeleter> typeInfo = makeTypeInfo<ReflectedTypeInfo>(typeName, typeSize, std::move(typeCaster), allowConstruction, placementConstructor, copyConstructor, moveConstructor, placementDestructor);
	if (allowCopyConstruction)
	{
		static_cast<ReflectedTypeInfo*>(typeInfo.get())->enableCopyConstruction();
	}
	if (allowMoveConstruction)
	{
		static_cast<ReflectedTypeInfo*>(typeInfo.get())->enableMoveConstruction();
	}
	static_cast<ReflectedTypeInfo*>(typeInfo.get())->setTriviallyCopyable(triviallyCopyable);
	static_cast<ReflectedTypeInfo*>(typeInfo.get())->setTriviallyConstructable(triviallyConstructable);
	return typeInfo;
}


std::unique_ptr<TypeInfo, TypeInfoDeleter> jitcat::Reflection::TypeRegistry::createEnumTypeInfo(const char* typeName, const CatGenericType& underlyingType, std::size_t typeSize, std::unique_ptr<TypeCaster> typeCaster)
{
	return makeTypeInfo<ReflectedEnumTypeInfo>(typeName, underlyingType, typeSize, std::move(typeCaster));
}


ReflectedTypeInfo* jitcat::Reflection::TypeRegistry::castToReflectedTypeInfo(TypeInfo* typeInfo)
{
	return static_cast<ReflectedTypeInfo*>(typeInfo);
}


ReflectedEnumTypeInfo* jitcat::Reflection::TypeRegistry::castToReflectedEnumTypeInfo(TypeInfo* typeInfo)
{
	return static_cast<ReflectedEnumTypeInfo*>(typeInfo);
}


TypeRegistry* TypeRegistry::instance = nullptr;
