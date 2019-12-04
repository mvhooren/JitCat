/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ContainerManipulator.h"
#include "jitcat/ArrayMemberFunctionInfo.h"
#include "jitcat/Configuration.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

#include <iostream>

using namespace jitcat;
using namespace jitcat::Reflection;


jitcat::Reflection::ArrayManipulator::ArrayManipulator(CatGenericType valueType):
	TypeInfo("array", sizeof(Array), std::make_unique<ObjectTypeCaster<Array>>()),
	valueType(valueType)
{
	memberFunctions.emplace("add", new ArrayTypeMemberFunctionInfo(ArrayTypeMemberFunctionInfo::Operation::Add, CatGenericType(this, false, false)));
	memberFunctions.emplace("remove", new ArrayTypeMemberFunctionInfo(ArrayTypeMemberFunctionInfo::Operation::Remove, CatGenericType(this, false, false)));
	memberFunctions.emplace("size", new ArrayTypeMemberFunctionInfo(ArrayTypeMemberFunctionInfo::Operation::Size, CatGenericType(this, false, false)));
	valueType.addDependentType(this);
	if (valueType.isBasicType()
		|| valueType.isPointerToReflectableObjectType()
		|| valueType.isReflectableHandleType())
	{
		assignableValueType = valueType.toPointer();
	}
}


jitcat::Reflection::ArrayManipulator::~ArrayManipulator()
{
}


std::size_t jitcat::Reflection::ArrayManipulator::getContainerSize(std::any container) const
{
	Array* array = std::any_cast<Array*>(container);
	return array->size;
}


std::any jitcat::Reflection::ArrayManipulator::getItemAt(std::any container, int index)
{
	Array* array = std::any_cast<Array*>(container);
	if (array != nullptr && index >= 0 && index < array->size)
	{
		unsigned char* itemPtr = &array->arrayData[index * valueType.getTypeSize()];
		return valueType.createAnyOfTypeAt(reinterpret_cast<uintptr_t>(itemPtr));
	}
	else
	{
		return valueType.createDefault();
	}
}


std::any jitcat::Reflection::ArrayManipulator::getAssignableItemAt(std::any container, int index)
{
	Array* array = std::any_cast<Array*>(container);
	if (array != nullptr && index >= 0 && index < array->size)
	{
		unsigned char* itemPtr = &array->arrayData[index * valueType.getTypeSize()];
		return assignableValueType.createAnyOfType(reinterpret_cast<uintptr_t>(itemPtr));
	}
	else
	{
		return assignableValueType.createDefault();
	}
}


std::any jitcat::Reflection::ArrayManipulator::getItemAt(std::any container, std::any key)
{
	return getItemAt(container, std::any_cast<int>(key));
}


int jitcat::Reflection::ArrayManipulator::getIndexOf(std::any container, std::any key)
{
	return -1;
}


std::any jitcat::Reflection::ArrayManipulator::createAnyPointer(uintptr_t pointer)
{
	return std::any(reinterpret_cast<Array*>(pointer));
}


std::any jitcat::Reflection::ArrayManipulator::getKeyAtIndex(std::any container, int index) const
{
	return std::any();
}


const CatGenericType& jitcat::Reflection::ArrayManipulator::getKeyType() const
{
	return CatGenericType::intType;
}


const CatGenericType& jitcat::Reflection::ArrayManipulator::getValueType() const
{
	return valueType;
}


const CatGenericType& jitcat::Reflection::ArrayManipulator::getAssignableValueType() const
{
	return assignableValueType;
}


int jitcat::Reflection::ArrayManipulator::add(Array* array, const std::any& value)
{
	if (array == nullptr)
	{
		return -1;
	}
	const unsigned char* valueBuffer = nullptr;
	std::size_t valueSize = 0;
	if (!valueType.isReflectableObjectType())
	{
		valueType.toBuffer(value, valueBuffer, valueSize);
	}
	else
	{
		valueType.toPointer(TypeOwnershipSemantics::Owned, false, false).toBuffer(value, valueBuffer, valueSize);
		valueSize = valueType.getTypeSize();
	}
	assert(valueSize == valueType.getTypeSize());
	if (array->size >= array->reserved)
	{
		//We need to resize the array, double the size each time
		int newReserved = array->reserved * 2;
		if (newReserved == 0)	newReserved = 1;
		unsigned char* newArrayBuffer = new unsigned char[newReserved * valueSize];
		if constexpr (Configuration::logJitCatObjectConstructionEvents)
		{
			std::cout << "(ArrayManipulator::add) Allocated buffer of size " << std::dec << newReserved * valueSize << ": " << std::hex << reinterpret_cast<uintptr_t>(newArrayBuffer) << "\n";
		}
		if (array->size > 0)
		{
			if (valueType.isTriviallyCopyable())
			{
				memcpy(newArrayBuffer, array->arrayData, array->size * valueSize);
			}
			else
			{
				assert(valueType.isMoveConstructible());
				for (int i = 0; i < array->size; i++)
				{
					valueType.moveConstruct(&newArrayBuffer[i * valueSize], valueSize, &array->arrayData[i * valueSize], valueSize);
				}
			}
		}
		delete[] array->arrayData;
		if constexpr (Configuration::logJitCatObjectConstructionEvents)
		{
			std::cout << "(ArrayManipulator::add) Deallocated buffer of size " << std::dec << array->reserved * valueSize << ": " << std::hex << reinterpret_cast<uintptr_t>(array->arrayData) << "\n";
		}
		array->arrayData = newArrayBuffer;
		array->reserved = newReserved;
	}
	unsigned char* targetLocation = &array->arrayData[valueSize * array->size];
	array->size++;
	if (valueType.isReflectableObjectType())
	{
		assert(valueType.isMoveConstructible());
		valueType.moveConstruct(targetLocation, valueSize, *reinterpret_cast<unsigned char**>(const_cast<unsigned char*>(valueBuffer)), valueSize);
	}
	else if (valueType.isPointerType() && valueType.getOwnershipSemantics() == TypeOwnershipSemantics::Owned)
	{
		assert(valueType.isMoveConstructible());
		valueType.moveConstruct(targetLocation, valueSize, const_cast<unsigned char*>(valueBuffer), valueSize);
	}
	else
	{
		assert(valueType.isCopyConstructible());
		valueType.copyConstruct(targetLocation, valueSize, valueBuffer, valueSize);
	}
	return array->size - 1;
}


void jitcat::Reflection::ArrayManipulator::remove(Array* array, int index)
{
	if (array == nullptr)
	{
		return;
	}
	std::size_t valueSize = valueType.getTypeSize();
	if (index >= array->size || index < 0)
	{
		return;
	}
	valueType.placementDestruct(&array->arrayData[index * valueSize], valueSize);
	if (index + 1 != array->size)
	{
		//We need to shift elements that came after index
		if (valueType.isTriviallyCopyable())
		{
			memmove(&array->arrayData[index * valueSize], &array->arrayData[index + 1 * valueSize], (array->size - index - 1) * valueSize);
		}
		else
		{
			assert(valueType.isMoveConstructible());
			for (int i = index + 1; i < array->size; i++)
			{
				valueType.moveConstruct(&array->arrayData[(i - 1) * valueSize], valueSize, &array->arrayData[i * valueSize], valueSize);
			}			
		}
	}
	array->size--;
}


bool jitcat::Reflection::ArrayManipulator::isArrayType() const
{
	return true;
}


void jitcat::Reflection::ArrayManipulator::placementConstruct(unsigned char* buffer, std::size_t bufferSize) const
{
	assert(bufferSize >= sizeof(Array));
	new(buffer) Array();
}


void jitcat::Reflection::ArrayManipulator::placementDestruct(unsigned char* buffer, std::size_t bufferSize)
{
	if (buffer == nullptr)
	{
		return;
	}
	Array* array = reinterpret_cast<Array*>(buffer);
	std::size_t typeSize = valueType.getTypeSize();
	if (array->arrayData != nullptr)
	{
		if (!valueType.isTriviallyCopyable())
		{
			for (int i = 0; i < array->size; i++)
			{
				valueType.placementDestruct(&array->arrayData[typeSize * i], typeSize);
			}
		}
		delete[] array->arrayData;
		if constexpr (Configuration::logJitCatObjectConstructionEvents)
		{
			std::cout << "(ArrayManipulator::placementDestruct) deallocated buffer of size " << std::dec << array->reserved * typeSize << ": " << std::hex << reinterpret_cast<uintptr_t>(array->arrayData) << "\n";
		}
		array->arrayData = nullptr;
		array->reserved = 0;
		array->size = 0;
	}
}


void jitcat::Reflection::ArrayManipulator::copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	assert(targetBufferSize >= sizeof(Array));
	assert(sourceBufferSize >= sizeof(Array));
	assert(targetBuffer != sourceBuffer);
	assert(valueType.isCopyConstructible());
	Array* target = new(targetBuffer) Array();
	
	const Array* source = reinterpret_cast<const Array*>(sourceBuffer);

	assert(target->arrayData == nullptr);
	std::size_t valueSize = valueType.getTypeSize();
	target->size = source->size;
	target->reserved = source->reserved;
	target->arrayData = new unsigned char[target->reserved * valueSize];

	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		std::cout << "(ArrayManipulator::copyConstruct) Allocated buffer of size " << std::dec << target->reserved * valueSize << ": " << std::hex << reinterpret_cast<uintptr_t>(target->arrayData) << "\n";
	}

	if (valueType.isTriviallyCopyable())
	{
		memcpy(target->arrayData, source->arrayData, target->size * valueSize);
	}
	else
	{
		assert(valueType.isMoveConstructible());
		for (int i = 0; i < target->size; i++)
		{
			valueType.copyConstruct(&target->arrayData[i * valueSize], valueSize, &source->arrayData[i * valueSize], valueSize);
		}	
	}
}


void jitcat::Reflection::ArrayManipulator::moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	assert(targetBufferSize >= sizeof(Array));
	assert(sourceBufferSize >= sizeof(Array));
	assert(targetBuffer != sourceBuffer);
	assert(valueType.isCopyConstructible());
	Array* target = reinterpret_cast<Array*>(targetBuffer);
	Array* source = reinterpret_cast<Array*>(sourceBuffer);

	assert(target->arrayData == nullptr);
	target->size = source->size;
	target->reserved = source->reserved;
	target->arrayData = source->arrayData;
	source->size = 0;
	source->reserved = 0;
	source->arrayData = nullptr;
}


bool jitcat::Reflection::ArrayManipulator::getAllowInheritance() const
{
	return false;
}


bool jitcat::Reflection::ArrayManipulator::inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext)
{
	return false;
}


bool jitcat::Reflection::ArrayManipulator::getAllowConstruction() const
{
	return true;
}


bool jitcat::Reflection::ArrayManipulator::getAllowCopyConstruction() const
{
	return true;
}


bool jitcat::Reflection::ArrayManipulator::getAllowMoveConstruction() const
{
	return true;
}


ArrayManipulator& jitcat::Reflection::ArrayManipulator::createArrayManipulatorOf(CatGenericType valueType)
{
	for (auto& iter : manipulators)
	{
		if (iter->getValueType().compare(valueType, true, true))
		{
			return *iter;
		}
	}
	ArrayManipulator* newManipilator = new ArrayManipulator(valueType);
	manipulators.push_back(newManipilator);
	return *newManipilator;
}


void jitcat::Reflection::ArrayManipulator::deleteArrayManipulatorsOfType(TypeInfo* objectType)
{
	for (int i = 0; i < manipulators.size(); i++)
	{
		if (manipulators[i]->getValueType().isDependentOn(objectType))
		{
			TypeInfo::destroy(manipulators[i]);
			manipulators.erase(manipulators.begin() + i);
			i--;
		}
	}
}


std::vector<ArrayManipulator*> jitcat::Reflection::ArrayManipulator::manipulators = std::vector<ArrayManipulator*>();