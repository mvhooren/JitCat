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

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>


namespace jitcat::Reflection
{
	class ReflectedTypeInfo;
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
		TypeInfo* getOrCreateTypeInfo(const char* typeName, std::size_t typeSize, TypeCaster* caster, std::function<Reflectable*()>& constructor,
									  std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
									  std::function<void (Reflectable*)>& destructor);

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
		static ReflectedTypeInfo* createTypeInfo(const char* typeName, std::size_t typeSize, TypeCaster* typeCaster, std::function<Reflectable*()>& constructor,
												std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
												std::function<void (Reflectable*)>& destructor);

	private:
		std::map<std::string, TypeInfo*> types;
		std::vector<std::unique_ptr<TypeInfo>> ownedTypes;
		static TypeRegistry* instance;
	};


	template<typename ReflectableT>
	inline jitcat::Reflection::TypeInfo* jitcat::Reflection::TypeRegistry::registerType()
	{
		jitcat::Reflection::TypeInfo* typeInfo = nullptr;
		//A compile error on this line usually means that there was an attempt to reflect a type that is not reflectable (or an unsupported basic type).
		const char* typeName = ReflectableT::getTypeName();
		std::string lowerTypeName = Tools::toLowerCase(typeName);
		std::map<std::string, TypeInfo*>::iterator iter = types.find(lowerTypeName);
		if (iter != types.end())
		{
			return iter->second;
		}
		else
		{

			std::function<jitcat::Reflection::Reflectable*()> constructor;
			std::function<void(unsigned char* buffer, std::size_t bufferSize)> placementConstructor;
			std::function<void (jitcat::Reflection::Reflectable*)> destructor;
			std::size_t typeSize = sizeof(ReflectableT);
			jitcat::Reflection::ObjectTypeCaster<ReflectableT>* typeCaster = new jitcat::Reflection::ObjectTypeCaster<ReflectableT>();

			if constexpr (std::is_default_constructible<ReflectableT>::value
							&& std::is_destructible<ReflectableT>::value)
			{
				constructor = [](){return new ReflectableT();};
				placementConstructor = [](unsigned char* buffer, std::size_t bufferSize) {assert(sizeof(ReflectableT) <= bufferSize); new(buffer) ReflectableT();};
			}
			else
			{
				constructor = [](){return nullptr;};
				placementConstructor = [](unsigned char* buffer, std::size_t bufferSize) {};
			}

			if constexpr (std::is_destructible<ReflectableT>::value)
			{
				destructor = [](jitcat::Reflection::Reflectable* object){delete static_cast<ReflectableT*>(object);};
			}
			else
			{
				destructor = [](jitcat::Reflection::Reflectable* object){};
			}

			jitcat::Reflection::ReflectedTypeInfo* reflectedInfo = createTypeInfo(typeName, typeSize, typeCaster, constructor, placementConstructor, destructor);
			typeInfo = reflectedInfo;
			types[lowerTypeName] = typeInfo;
			ownedTypes.emplace_back(typeInfo);
			ReflectableT::reflect(*reflectedInfo);
			return typeInfo;
		}
	}


} //End namespace jitcat::Reflection