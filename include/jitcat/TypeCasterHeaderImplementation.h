/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/
#pragma once

#include "jitcat/STLTypeReflectors.h"
#include "jitcat/TypeTools.h"


namespace jitcat::Reflection
{
	template<typename ObjectT>
	inline bool ObjectTypeCaster<ObjectT>::isNullPtr(const std::any& value) const
	{
		ObjectT* ptr = std::any_cast<ObjectT*>(value);
		return ptr == nullptr;
	}


	template<typename ObjectT>
	inline bool ObjectTypeCaster<ObjectT>::isNullPtrPtr(const std::any& value) const
	{
		ObjectT** ptr = std::any_cast<ObjectT**>(value);
		return ptr == nullptr;
	}


	template<typename ObjectT>
	inline std::any ObjectTypeCaster<ObjectT>::getValueOfPointer(std::any& value) const
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


	template<typename ObjectT>
	inline std::any ObjectTypeCaster<ObjectT>::getValueOfPointerToPointer(std::any& value) const
	{
		ObjectT** ptrptr = std::any_cast<ObjectT**>(value);
		return *ptrptr;
	}


	template<typename ObjectT>
	inline std::any ObjectTypeCaster<ObjectT>::getAddressOfValue(std::any& value) const
	{
		if constexpr (TypeTools::getAllowCopyConstruction<ObjectT>())
		{
			//We should always get here because a std::any can only contain copy-constructible values.
			ObjectT* addressOf = std::any_cast<ObjectT>(&value);
			return addressOf;
		}
		else
		{
			assert(false);
			return nullptr;
		}
	}


	template<typename ObjectT>
	inline std::any ObjectTypeCaster<ObjectT>::getAddressOfPointer(std::any& value) const
	{
		ObjectT** addressOf = std::any_cast<ObjectT*>(&value);
		return addressOf;
	}


	template<typename ObjectT>
	inline std::any ObjectTypeCaster<ObjectT>::castFromRawPointer(uintptr_t pointer) const
	{
		return reinterpret_cast<ObjectT*>(pointer);
	}


	template<typename ObjectT>
	inline uintptr_t ObjectTypeCaster<ObjectT>::castToRawPointer(const std::any& pointer) const
	{
		return reinterpret_cast<uintptr_t>(std::any_cast<ObjectT*>(pointer));
	}


	template<typename ObjectT>
	inline std::any ObjectTypeCaster<ObjectT>::castFromRawPointerPointer(uintptr_t pointer) const
	{
		return reinterpret_cast<ObjectT**>(pointer);
	}


	template<typename ObjectT>
	inline uintptr_t ObjectTypeCaster<ObjectT>::castToRawPointerPointer(const std::any& pointer) const
	{
		return reinterpret_cast<uintptr_t>(std::any_cast<ObjectT**>(pointer));
	}


	template<typename ObjectT>
	inline void ObjectTypeCaster<ObjectT>::toBuffer(const std::any& value, const unsigned char*& buffer, std::size_t& bufferSize) const
	{
		ObjectT* object = std::any_cast<ObjectT*>(value);
		buffer = reinterpret_cast<const unsigned char*>(object);
		bufferSize = sizeof(ObjectT);
	}


	template<typename ObjectT>
	inline std::any ObjectTypeCaster<ObjectT>::getNull() const
	{
		return (ObjectT*)nullptr;
	}

} //End namespace jitcat::Reflection