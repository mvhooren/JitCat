/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once
#include "jitcat/Reflectable.h"
#include "jitcat/STLTypeReflectors.h"
#include "jitcat/TypeTools.h"

#include <any>
#include <cassert>
#include <map>
#include <string>
#include <type_traits>
#include <vector>


namespace jitcat::Reflection
{
	class CustomTypeInfo;

	class TypeCaster
	{
	public:
		TypeCaster() {};
		virtual ~TypeCaster() {};
		virtual bool isNullPtr(const std::any& value) const = 0;
		virtual bool isNullPtrPtr(const std::any& value) const = 0;
		virtual std::any getValueOfPointer(std::any& value) const = 0;
		virtual std::any getValueOfPointerToPointer(std::any& value) const = 0;
		virtual std::any getAddressOfValue(std::any& value) const = 0;
		virtual std::any getAddressOfPointer(std::any& value) const = 0;
		virtual std::any castFromRawPointer(uintptr_t pointer) const = 0;
		virtual uintptr_t castToRawPointer(const std::any& pointer) const = 0;
		virtual std::any castFromRawPointerPointer(uintptr_t pointer) const = 0;
		virtual uintptr_t castToRawPointerPointer(const std::any& pointer) const = 0;

		virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const = 0;
		virtual std::any getNull() const = 0;
	};


	template<typename ObjectT>
	class ObjectTypeCaster: public TypeCaster
	{
	public:
		ObjectTypeCaster() {};
		virtual ~ObjectTypeCaster() {};

		virtual bool isNullPtr(const std::any& value) const override final
		{
			ObjectT* ptr = std::any_cast<ObjectT*>(value);
			return ptr == nullptr;
		}


		virtual bool isNullPtrPtr(const std::any& value) const override final
		{
			ObjectT** ptr = std::any_cast<ObjectT**>(value);
			return ptr == nullptr;
		}


		inline virtual std::any getValueOfPointer(std::any& value) const override final
		{
			if constexpr (TypeTools::getAllowCopyConstruction<ObjectT>())
			{
				ObjectT* ptr = std::any_cast<ObjectT*>(value);
				std::any result(std::in_place_type<ObjectT>, *ptr);
				return result;
			}
			else
			{
				assert(false);
				return nullptr;
			}
		}


		virtual std::any getValueOfPointerToPointer(std::any& value) const override final
		{
			ObjectT** ptrptr = std::any_cast<ObjectT**>(value);
			return *ptrptr;
		}


		virtual std::any getAddressOfValue(std::any& value) const override final
		{
			ObjectT* addressOf = std::any_cast<ObjectT>(&value);
			return addressOf;
		}


		virtual std::any getAddressOfPointer(std::any& value) const override final
		{
			ObjectT** addressOf = std::any_cast<ObjectT*>(&value);
			return addressOf;
		}


		virtual std::any castFromRawPointer(uintptr_t pointer) const override final
		{
			return reinterpret_cast<ObjectT*>(pointer);
		}


		virtual uintptr_t castToRawPointer(const std::any& pointer) const override final
		{
			return reinterpret_cast<uintptr_t>(std::any_cast<ObjectT*>(pointer));
		}


		virtual std::any castFromRawPointerPointer(uintptr_t pointer) const override final
		{
			return reinterpret_cast<ObjectT**>(pointer);
		}


		virtual uintptr_t castToRawPointerPointer(const std::any& pointer) const override final
		{
			return reinterpret_cast<uintptr_t>(std::any_cast<ObjectT**>(pointer));
		}


		virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const override final
		{
			ObjectT* object = std::any_cast<ObjectT*>(value);
			buffer = reinterpret_cast<const unsigned char*>(object);
			bufferSize = sizeof(ObjectT);
		}


		virtual std::any getNull() const override final 
		{
			return (ObjectT*)nullptr;
		}

	};


class CustomObjectTypeCaster: public TypeCaster
{
public:
	CustomObjectTypeCaster(CustomTypeInfo* customType);
	virtual ~CustomObjectTypeCaster() {};

	virtual bool isNullPtr(const std::any& value) const override final;
	virtual bool isNullPtrPtr(const std::any& value) const override final;
	virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const override final;
	virtual std::any getValueOfPointer(std::any& value) const override final;
	virtual std::any getValueOfPointerToPointer(std::any& value) const override final;
	virtual std::any getAddressOfValue(std::any& value) const override final;
	virtual std::any getAddressOfPointer(std::any& value) const override final;
	virtual std::any castFromRawPointer(uintptr_t pointer) const override final;
	virtual uintptr_t castToRawPointer(const std::any& pointer) const override final;
	virtual std::any castFromRawPointerPointer(uintptr_t pointer) const override final;
	virtual uintptr_t castToRawPointerPointer(const std::any& pointer) const override final;
	virtual std::any getNull() const override final;

private:
	CustomTypeInfo* customType;
};


class NullptrTypeCaster: public TypeCaster
{
public:
	NullptrTypeCaster() {};
	virtual ~NullptrTypeCaster() {};
	
	virtual bool isNullPtr(const std::any& value) const override final;
	virtual bool isNullPtrPtr(const std::any& value) const override final;
	virtual std::any getValueOfPointer(std::any& value) const override final;
	virtual std::any getValueOfPointerToPointer(std::any& value) const override final;
	virtual std::any getAddressOfValue(std::any& value) const override final;
	virtual std::any getAddressOfPointer(std::any& value) const override final;
	virtual std::any castFromRawPointer(uintptr_t pointer) const override final;
	virtual uintptr_t castToRawPointer(const std::any& pointer) const override final;
	virtual std::any castFromRawPointerPointer(uintptr_t pointer) const override final;
	virtual uintptr_t castToRawPointerPointer(const std::any& pointer) const override final;
	virtual void toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const override final;
	virtual std::any getNull() const override final;
};


} //End namespace jitcat::Reflection