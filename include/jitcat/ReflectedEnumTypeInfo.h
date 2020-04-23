/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include "jitcat/CatGenericType.h"
#include "jitcat/TypeInfo.h"

#include <cassert>


namespace jitcat::Reflection
{
	class Reflectable;
	class StaticConstMemberInfo;
	class TypeCaster;

	//ReflectedEnumTypeInfo represents type information about a C++ enum that is made to be Reflectable.
	//To make an enum reflectable, two functions need to be defined in the global scope:
	//
	//A function that allows reflection to get a name for the enum.
	//template <>
	//const char* jitcat::Reflection::getEnumName<YourEnum>();
	//
	//And a function that adds all the enum values to the enumTypeInfo via enumTypeInfo.addValue("EnumValueName", YourEnum::YourEnumValue)
	//template <>
	//void jitcat::Reflection::reflectEnum<YourEnum>(jitcat::Reflection::ReflectedEnumTypeInfo& enumTypeInfo);
	class ReflectedEnumTypeInfo: public TypeInfo
	{
	public:
		ReflectedEnumTypeInfo(const char* typeName, const CatGenericType& underlyingType, std::size_t typeSize, std::unique_ptr<TypeCaster> caster);
		virtual ~ReflectedEnumTypeInfo();

		template<typename EnumT>
		inline ReflectedEnumTypeInfo& addValue(const char* valueName, EnumT value)
		{
			assert(getEnumName<EnumT>() == getTypeName());
			const CatGenericType& enumType = TypeTraits<EnumT>::toGenericType();
			StaticConstMemberInfo* memberInfo = addConstant(valueName, enumType, value);
			if (!defaultUnderlyingValue.has_value())
			{
				setDefaultValue(value);
			}
			return *this;
		}

		//When this enum is default constructed, it is initialised with the value specified.
		//If no default value is set this way, the default value will be the first value added with addValue.
		template<typename EnumT>
		inline ReflectedEnumTypeInfo& setDefaultValue(EnumT value)
		{
			assert(getEnumName<EnumT>() == getTypeName());
			defaultUnderlyingValue = static_cast<typename std::underlying_type_t<EnumT>>(value);
			underlyingType.toBuffer(defaultUnderlyingValue, defaultValueBuffer, defaultValueSize);
			return *this;
		}

		//Construct or destruct instances of the type (if allowed)
		virtual void placementConstruct(unsigned char* buffer, std::size_t bufferSize) const override final;
		virtual void placementDestruct(unsigned char* buffer, std::size_t bufferSize) override final;
		virtual void copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;
		virtual void moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize) override final;

		virtual bool getAllowInheritance() const override final;
		virtual bool inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext) override final;
		virtual bool getAllowConstruction() const override final;
		virtual bool getAllowCopyConstruction() const override final;
		virtual bool getAllowMoveConstruction() const override final;

		virtual bool isTriviallyCopyable() const override final;

	private:
		CatGenericType underlyingType;
		std::any defaultUnderlyingValue;
		const unsigned char* defaultValueBuffer;
		std::size_t defaultValueSize;
	};
};