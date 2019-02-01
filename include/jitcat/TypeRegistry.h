/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

namespace jitcat
{
	class CatGenericType;
}
#include "jitcat/Tools.h"
#include "jitcat/TypeCaster.h"

#include <map>
#include <memory>
#include <string>


namespace jitcat::Reflection
{
	class TypeInfo;


	class TypeRegistry
	{
	private:
		TypeRegistry();
		TypeRegistry(const TypeRegistry&);
		~TypeRegistry();

	public:
		static TypeRegistry* get();
		static void recreate();

		//Returns nullptr if type wasn't found, type names are case sensitive
		TypeInfo* getTypeInfo(const std::string& typeName);
		//Never returns nullptr, creates a new empty TypeInfo if typeName does not exist.
		TypeInfo* getOrCreateTypeInfo(const char* typeName, TypeCaster* caster);

		const std::map<std::string, TypeInfo*>& getTypes() const;
	
		//If the type is already registered, it will just return the TypeInfo.
		//Never returns nullptr
		template<typename T>
		TypeInfo* registerType();
		void registerType(const char* typeName, TypeInfo* typeInfo);

		void removeType(const char* typeName);
		void renameType(const std::string& oldName, const char* newTypeName);

		//Type registry loaded this way is not sutable for executing expressions, only for expression syntax and type checking
		bool loadRegistryFromXML(const std::string& filepath);
		//Exports all registered types to XML. Intended for use in external tools.
		void exportRegistyToXML(const std::string& filepath);

	private:
		//This function exists to prevent circular includes via TypeInfo.h
		static TypeInfo* createTypeInfo(const char* typeName, TypeCaster* typeCaster);

	private:
		std::map<std::string, TypeInfo*> types;
		std::vector<std::unique_ptr<TypeInfo>> ownedTypes;
		static TypeRegistry* instance;
	};


	template<typename T>
	inline TypeInfo* TypeRegistry::registerType()
	{
		TypeInfo* typeInfo = nullptr;
		//A compile error on this line usually means that there was an attempt to reflect a type that is not reflectable (or an unsupported basic type).
		const char* typeName = T::getTypeName();
		std::map<std::string, TypeInfo*>::iterator iter = types.find(typeName);
		if (iter != types.end())
		{
			return iter->second;
		}
		else
		{
			typeInfo = createTypeInfo(typeName, new ObjectTypeCaster<T>());
			types[typeName] = typeInfo;
			ownedTypes.emplace_back(typeInfo);
			T::reflect(*typeInfo);
			return typeInfo;
		}
	}

} //End namespace jitcat::Reflection
