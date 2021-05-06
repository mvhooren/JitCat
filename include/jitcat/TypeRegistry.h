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
#include "jitcat/TypeInfoDeleter.h"

#include <cassert>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>


namespace jitcat::Reflection
{
	class ReflectedTypeInfo;
	class ReflectedEnumTypeInfo;
	class TypeCaster;
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

		const std::map<std::string, TypeInfo*>& getTypes() const;
	
		//If the type is already registered, it will just return the TypeInfo.
		//Never returns nullptr
		template<typename T>
		TypeInfo* registerType(TypeInfo** typeInfoToSet = nullptr);
		void registerType(const char* typeName, TypeInfo* typeInfo);

		void removeType(const char* typeName);
		void renameType(const std::string& oldName, const char* newTypeName);

		//Type registry loaded this way is not sutable for executing expressions, only for expression syntax and type checking
		bool loadRegistryFromXML(const std::string& filepath);
		//Exports all registered types to XML. Intended for use in external tools.
		void exportRegistyToXML(const std::string& filepath);

	private:
		//This function exists to prevent circular includes via TypeInfo.h
		static std::unique_ptr<TypeInfo, TypeInfoDeleter> createTypeInfo(const char* typeName, std::size_t typeSize, std::unique_ptr<TypeCaster> typeCaster, bool allowConstruction, 
												 bool allowCopyConstruction, bool allowMoveConstruction, bool triviallyCopyable,
												 std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor,
												 std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& copyConstructor,
												 std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& moveConstructor,
												 std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor);
		static std::unique_ptr<TypeInfo, TypeInfoDeleter> createEnumTypeInfo(const char* typeName, const CatGenericType& underlyingType, std::size_t typeSize, std::unique_ptr<TypeCaster> typeCaster);
		static ReflectedTypeInfo* castToReflectedTypeInfo(TypeInfo* typeInfo);
		static ReflectedEnumTypeInfo* castToReflectedEnumTypeInfo(TypeInfo* typeInfo);
	private:
		std::map<std::string, TypeInfo*> types;
		std::vector<std::unique_ptr<TypeInfo, TypeInfoDeleter>> ownedTypes;
		static TypeRegistry* instance;
	};


}

#include "jitcat/STLTypeReflectors.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/TypeTools.h"
#include "jitcat/TypeTraits.h"

namespace jitcat::Reflection
{

	template<typename ReflectableCVT>
	inline jitcat::Reflection::TypeInfo* jitcat::Reflection::TypeRegistry::registerType(TypeInfo** typeInfoToSet)
	{
		using ReflectableT = typename RemoveConst<ReflectableCVT>::type;
		
		//A compile error on this line usually means that there was an attempt to reflect a type that is not reflectable (or an unsupported basic type).
		const char* typeName = TypeNameGetter<ReflectableT>::get();
		std::string lowerTypeName = Tools::toLowerCase(typeName);
		std::map<std::string, TypeInfo*>::iterator iter = types.find(lowerTypeName);
		if (iter != types.end())
		{
			if (typeInfoToSet != nullptr)
			{
				*typeInfoToSet = iter->second;
			}
			return iter->second;
		}
		else
		{
			std::size_t typeSize = sizeof(ReflectableT);
			std::unique_ptr<jitcat::Reflection::ObjectTypeCaster<ReflectableT>> typeCaster = std::make_unique<jitcat::Reflection::ObjectTypeCaster<ReflectableT>>();

			if constexpr (!std::is_enum_v<ReflectableT>)
			{
				std::function<void(unsigned char* buffer, std::size_t bufferSize)> placementConstructor;
				std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)> copyConstructor;
				std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)> moveConstructor;
				std::function<void(unsigned char* buffer, std::size_t bufferSize)> placementDestructor;

				constexpr bool isConstructible = std::is_default_constructible<ReflectableT>::value
												 && std::is_destructible<ReflectableT>::value;
				if constexpr (isConstructible)
				{
					placementConstructor = [](unsigned char* buffer, std::size_t bufferSize) {assert(sizeof(ReflectableT) <= bufferSize); new(buffer) ReflectableT();};
				}
				else
				{
					placementConstructor = [](unsigned char* buffer, std::size_t bufferSize) {};
				}
				constexpr bool isCopyConstructible = TypeTools::getAllowCopyConstruction<ReflectableT>();
				if constexpr (isCopyConstructible)
				{
					copyConstructor = [](unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
									  {
										new(targetBuffer) ReflectableT(*reinterpret_cast<const ReflectableT*>(sourceBuffer));
									  };
				}
				else
				{
					copyConstructor = [](unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize) {};
				}

				constexpr bool isMoveConstructible = std::is_move_constructible<ReflectableT>::value;
				if constexpr (isMoveConstructible)
				{
					moveConstructor = [](unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
									  {
										new(targetBuffer) ReflectableT(std::move(*reinterpret_cast<ReflectableT*>(sourceBuffer)));
									  };
				}
				else
				{
					moveConstructor = [](unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize) {};
				}

				if constexpr (std::is_destructible<ReflectableT>::value)
				{
					placementDestructor = [](unsigned char* buffer, std::size_t bufferSize){reinterpret_cast<ReflectableT*>(buffer)->~ReflectableT();};
				}
				else
				{
					placementDestructor = [](unsigned char* buffer, std::size_t bufferSize){};
				}

				//When a type within JitCat is triviallyCopyable it implies that it is also trivially destructible.
				//This is not always true for C++ types and so we also need to check for is_trivially_destructible.
				constexpr bool triviallyCopyable = std::is_trivially_copyable<ReflectableT>::value && std::is_trivially_destructible<ReflectableT>::value;

				
				std::unique_ptr<jitcat::Reflection::TypeInfo, TypeInfoDeleter> typeInfo = createTypeInfo(typeName, typeSize, std::move(typeCaster), isConstructible, isCopyConstructible || triviallyCopyable, isMoveConstructible || triviallyCopyable, triviallyCopyable,
																					  placementConstructor, copyConstructor, moveConstructor, placementDestructor);
				types[lowerTypeName] = typeInfo.get();
				if (typeInfoToSet != nullptr)
				{
					*typeInfoToSet = typeInfo.get();
				}
				jitcat::Reflection::TypeInfo* returnTypeInfo = typeInfo.get();
				ownedTypes.emplace_back(std::move(typeInfo));
				if constexpr (GetTypeNameAndReflectExist<ReflectableT>::value)
				{
					ReflectableT::reflect(*castToReflectedTypeInfo(returnTypeInfo));
				}
				else if constexpr (ExternalReflector<ReflectableT>::exists)
				{
					ExternalReflector<ReflectableT>::reflect(*castToReflectedTypeInfo(returnTypeInfo));
				}
				else
				{
					static_assert(ExternalReflector<ReflectableT>::exists, "Need to implement reflection for ReflectableT");
				}
				
				return returnTypeInfo;
			}
			else
			{
				std::unique_ptr<jitcat::Reflection::TypeInfo, TypeInfoDeleter> typeInfo = createEnumTypeInfo(typeName, TypeTraits<typename UnderlyingType<ReflectableT>::type>::toGenericType(), typeSize, std::move(typeCaster));
				types[lowerTypeName] = typeInfo.get();
				jitcat::Reflection::TypeInfo* returnTypeInfo = typeInfo.get();
				ownedTypes.emplace_back(std::move(typeInfo));
				reflectEnum<ReflectableT>(*castToReflectedEnumTypeInfo(returnTypeInfo));
				return returnTypeInfo;
			}
		}
	}


} //End namespace jitcat::Reflection
