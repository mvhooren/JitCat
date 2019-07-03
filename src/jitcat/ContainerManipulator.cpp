/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ContainerManipulator.h"
#include "jitcat/ArrayMemberFunctionInfo.h"
#include "jitcat/ReflectableHandle.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


jitcat::Reflection::ArrayManipulator::ArrayManipulator(CatGenericType valueType):
	valueType(valueType),
	addFunction(new ArrayTypeMemberFunctionInfo(ArrayTypeMemberFunctionInfo::Operation::Add, CatGenericType(ContainerType::Array, this, false, false))),
	removeFunction(new ArrayTypeMemberFunctionInfo(ArrayTypeMemberFunctionInfo::Operation::Remove, CatGenericType(ContainerType::Array, this, false, false))),
	sizeFunction(new ArrayTypeMemberFunctionInfo(ArrayTypeMemberFunctionInfo::Operation::Size, CatGenericType(ContainerType::Array, this, false, false)))
{
}


std::size_t jitcat::Reflection::ArrayManipulator::getContainerSize(std::any container) const
{
	Array* array = static_cast<Array*>(std::any_cast<Reflectable*>(container));
	return array->size;
}


std::any jitcat::Reflection::ArrayManipulator::getItemAt(std::any container, int index)
{
	Array* array = static_cast<Array*>(std::any_cast<Reflectable*>(container));
	if (index >= 0 && index < array->size)
	{
		unsigned char* itemPtr = &array->arrayData[index * valueType.getTypeSize()];
		return valueType.createAnyOfTypeAt(reinterpret_cast<uintptr_t>(itemPtr));
	}
	else
	{
		return valueType.createDefault();
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
	return std::any(reinterpret_cast<Reflectable*>(pointer));
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


MemberFunctionInfo* jitcat::Reflection::ArrayManipulator::getMemberFunctionInfo(const std::string& functionName)
{
	std::string lowerName = Tools::toLowerCase(functionName);
		 if (lowerName == "add")	return getAddFunction();
	else if (lowerName == "remove")	return getRemoveFunction();
	else if (lowerName == "size")	return getSizeFunction();
	else							return nullptr;
}


int jitcat::Reflection::ArrayManipulator::add(Array* array, const std::any& value)
{
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


void jitcat::Reflection::ArrayManipulator::placementConstruct(unsigned char* buffer, std::size_t bufferSize)
{
	assert(bufferSize >= sizeof(Array));
	new(buffer) Array();
}


void jitcat::Reflection::ArrayManipulator::placementDestruct(unsigned char* buffer, std::size_t bufferSize)
{
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
		array->arrayData = nullptr;
		array->reserved = 0;
		array->size = 0;
	}
}


void jitcat::Reflection::ArrayManipulator::copy(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	assert(targetBufferSize >= sizeof(Array));
	assert(sourceBufferSize >= sizeof(Array));
	assert(targetBuffer != sourceBuffer);
	assert(valueType.isCopyConstructible());
	Array* target = reinterpret_cast<Array*>(targetBuffer);
	const Array* source = reinterpret_cast<const Array*>(sourceBuffer);

	assert(target->arrayData == nullptr);
	std::size_t valueSize = valueType.getTypeSize();
	target->size = source->size;
	target->reserved = source->reserved;
	target->arrayData = new unsigned char[target->reserved * valueSize];
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


void jitcat::Reflection::ArrayManipulator::move(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
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


ArrayManipulator& jitcat::Reflection::ArrayManipulator::createArrayManipulatorOf(CatGenericType valueType)
{
	for (auto& iter : manipulators)
	{
		if (iter->getValueType().compare(valueType, true))
		{
			return *iter.get();
		}
	}
	ArrayManipulator* newManipilator = new ArrayManipulator(valueType);
	manipulators.emplace_back(newManipilator);
	return *newManipilator;
}


void jitcat::Reflection::ArrayManipulator::deleteArrayManipulatorsOfType(TypeInfo* objectType)
{
	for (int i = 0; i < manipulators.size(); i++)
	{
		if (manipulators[i]->getValueType().isDependentOn(objectType))
		{
			manipulators.erase(manipulators.begin() + i);
			i--;
		}
	}
}


ArrayTypeMemberFunctionInfo* jitcat::Reflection::ArrayManipulator::getAddFunction()
{
	return addFunction.get();
}


ArrayTypeMemberFunctionInfo* jitcat::Reflection::ArrayManipulator::getRemoveFunction()
{
	return removeFunction.get();
}


ArrayTypeMemberFunctionInfo* jitcat::Reflection::ArrayManipulator::getSizeFunction()
{
	return sizeFunction.get();;
}


std::vector<std::unique_ptr<ArrayManipulator>> jitcat::Reflection::ArrayManipulator::manipulators = std::vector<std::unique_ptr<ArrayManipulator>>();