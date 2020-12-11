#include "jitcat/ArrayTypeInfo.h"
#include "jitcat/ArrayMemberFunctionInfo.h"
#include "jitcat/TypeCaster.h"
#include <cassert>
#include <iostream>


using namespace jitcat;
using namespace jitcat::Reflection;


ArrayTypeInfo::ArrayTypeInfo(CatGenericType arrayItemType):
	TypeInfo("array", sizeof(Array), std::make_unique<ObjectTypeCaster<Array>>()),
	arrayItemType(arrayItemType)
{
	memberFunctions.emplace("add", std::make_unique<ArrayMemberFunctionInfo>(ArrayMemberFunctionInfo::Operation::Add, this));
	memberFunctions.emplace("remove", std::make_unique<ArrayMemberFunctionInfo>(ArrayMemberFunctionInfo::Operation::Remove, this));
	memberFunctions.emplace("size", std::make_unique<ArrayMemberFunctionInfo>(ArrayMemberFunctionInfo::Operation::Size, this));
	memberFunctions.emplace("[]", std::make_unique<ArrayMemberFunctionInfo>(ArrayMemberFunctionInfo::Operation::Index, this));
	arrayItemType.addDependentType(this);
}


ArrayTypeInfo::~ArrayTypeInfo()
{
}


int ArrayTypeInfo::add(Array* array, const std::any& value)
{
	if (array == nullptr)
	{
		return -1;
	}
	const unsigned char* valueBuffer = nullptr;
	std::size_t valueSize = 0;
	if (!arrayItemType.isReflectableObjectType())
	{
		arrayItemType.toBuffer(value, valueBuffer, valueSize);
	}
	else
	{
		arrayItemType.toPointer(TypeOwnershipSemantics::Owned, false, false).toBuffer(value, valueBuffer, valueSize);
		valueSize = arrayItemType.getTypeSize();
	}
	assert(valueSize == arrayItemType.getTypeSize());
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
			if (arrayItemType.isTriviallyCopyable())
			{
				memcpy(newArrayBuffer, array->arrayData, array->size * valueSize);
			}
			else
			{
				assert(arrayItemType.isMoveConstructible());
				for (int i = 0; i < array->size; i++)
				{
					arrayItemType.moveConstruct(&newArrayBuffer[i * valueSize], valueSize, &array->arrayData[i * valueSize], valueSize);
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
	if (arrayItemType.isReflectableObjectType())
	{
		assert(arrayItemType.isMoveConstructible());
		arrayItemType.moveConstruct(targetLocation, valueSize, *reinterpret_cast<unsigned char**>(const_cast<unsigned char*>(valueBuffer)), valueSize);
	}
	else if (arrayItemType.isPointerType() && arrayItemType.getOwnershipSemantics() == TypeOwnershipSemantics::Owned)
	{
		assert(arrayItemType.isMoveConstructible());
		arrayItemType.moveConstruct(targetLocation, valueSize, const_cast<unsigned char*>(valueBuffer), valueSize);
	}
	else
	{
		assert(arrayItemType.isCopyConstructible());
		arrayItemType.copyConstruct(targetLocation, valueSize, valueBuffer, valueSize);
	}
	return array->size - 1;
}


void ArrayTypeInfo::remove(Array* array, int index)
{
	if (array == nullptr)
	{
		return;
	}
	std::size_t valueSize = arrayItemType.getTypeSize();
	if (index >= array->size || index < 0)
	{
		return;
	}
	arrayItemType.placementDestruct(&array->arrayData[index * valueSize], valueSize);
	if (index + 1 != array->size)
	{
		//We need to shift elements that came after index
		if (arrayItemType.isTriviallyCopyable())
		{
			memmove(&array->arrayData[index * valueSize], &array->arrayData[index + 1 * valueSize], (array->size - index - 1) * valueSize);
		}
		else
		{
			assert(arrayItemType.isMoveConstructible());
			for (int i = index + 1; i < array->size; i++)
			{
				arrayItemType.moveConstruct(&array->arrayData[(i - 1) * valueSize], valueSize, &array->arrayData[i * valueSize], valueSize);
			}			
		}
	}
	array->size--;
}


std::any ArrayTypeInfo::index(Array* array, int index)
{
	if (array != nullptr && index >= 0 && index < array->size)
	{
		std::size_t itemSize = arrayItemType.getTypeSize();
		uintptr_t itemAddress = reinterpret_cast<uintptr_t>(array->arrayData + index * itemSize);
		return arrayItemType.createAnyOfType(itemAddress);
	}
	else
	{
		return arrayItemType.createDefault();
	}
}


bool ArrayTypeInfo::isArrayType() const
{
	return true;
}


void ArrayTypeInfo::placementConstruct(unsigned char* buffer, std::size_t bufferSize) const
{
	assert(bufferSize >= sizeof(Array));
	new(buffer) Array();
}


void ArrayTypeInfo::placementDestruct(unsigned char* buffer, std::size_t bufferSize)
{
	if (buffer == nullptr)
	{
		return;
	}
	Array* array = reinterpret_cast<Array*>(buffer);
	std::size_t typeSize = arrayItemType.getTypeSize();
	if (array->arrayData != nullptr)
	{
		if (!arrayItemType.isTriviallyCopyable())
		{
			for (int i = 0; i < array->size; i++)
			{
				arrayItemType.placementDestruct(&array->arrayData[typeSize * i], typeSize);
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


void ArrayTypeInfo::copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	assert(targetBufferSize >= sizeof(Array));
	assert(sourceBufferSize >= sizeof(Array));
	assert(targetBuffer != sourceBuffer);
	assert(arrayItemType.isCopyConstructible());
	Array* target = new(targetBuffer) Array();
	
	const Array* source = reinterpret_cast<const Array*>(sourceBuffer);

	assert(target->arrayData == nullptr);
	std::size_t valueSize = arrayItemType.getTypeSize();
	target->size = source->size;
	target->reserved = source->reserved;
	target->arrayData = new unsigned char[target->reserved * valueSize];

	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		std::cout << "(ArrayManipulator::copyConstruct) Allocated buffer of size " << std::dec << target->reserved * valueSize << ": " << std::hex << reinterpret_cast<uintptr_t>(target->arrayData) << "\n";
	}

	if (arrayItemType.isTriviallyCopyable())
	{
		memcpy(target->arrayData, source->arrayData, target->size * valueSize);
	}
	else
	{
		assert(arrayItemType.isCopyConstructible());
		for (int i = 0; i < target->size; i++)
		{
			arrayItemType.copyConstruct(&target->arrayData[i * valueSize], valueSize, &source->arrayData[i * valueSize], valueSize);
		}	
	}
}


void ArrayTypeInfo::moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	assert(targetBufferSize >= sizeof(Array));
	assert(sourceBufferSize >= sizeof(Array));
	assert(targetBuffer != sourceBuffer);
	assert(arrayItemType.isCopyConstructible());
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


bool ArrayTypeInfo::getAllowInheritance() const
{
	return false;
}


bool ArrayTypeInfo::inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext)
{
	return false;
}


bool ArrayTypeInfo::getAllowConstruction() const
{
	return true;
}


bool ArrayTypeInfo::getAllowCopyConstruction() const
{
	return true;
}


bool ArrayTypeInfo::getAllowMoveConstruction() const
{
	return true;
}


ArrayTypeInfo& ArrayTypeInfo::createArrayTypeOf(CatGenericType arrayItemType)
{
	for (auto& iter : arrayTypes)
	{
		if (iter->getArrayItemType().compare(arrayItemType, true, true))
		{
			return *iter;
		}
	}
	ArrayTypeInfo* newArrayType = new ArrayTypeInfo(arrayItemType);
	arrayTypes.push_back(newArrayType);
	return *newArrayType;
}


void ArrayTypeInfo::deleteArrayTypeOfType(CatGenericType arrayItemType)
{
	for (int i = 0; i < arrayTypes.size(); i++)
	{
		if (arrayTypes[i]->getArrayItemType().compare(arrayItemType, true, true))
		{
			TypeInfo::destroy(arrayTypes[i]);
			arrayTypes.erase(arrayTypes.begin() + i);
			i--;
		}
	}
}


const CatGenericType& ArrayTypeInfo::getArrayItemType() const
{
	return arrayItemType;
}


std::vector<ArrayTypeInfo*> ArrayTypeInfo::arrayTypes = std::vector<ArrayTypeInfo*>();