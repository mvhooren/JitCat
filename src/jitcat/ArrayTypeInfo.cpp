#include "jitcat/ArrayTypeInfo.h"
#include "jitcat/ArrayMemberFunctionInfo.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/LLVMCatIntrinsics.h"

#include <cassert>
#include <iostream>


using namespace jitcat;
using namespace jitcat::Reflection;


ArrayTypeInfo::ArrayTypeInfo(CatGenericType arrayItemType):
	TypeInfo("array", sizeof(Array), std::make_unique<ObjectTypeCaster<Array>>()),
	arrayItemType(arrayItemType)
{
	memberFunctions.emplace("__init", std::make_unique<ArrayMemberFunctionInfo>(ArrayMemberFunctionInfo::Operation::Init, this));
	memberFunctions.emplace("__destroy", std::make_unique<ArrayMemberFunctionInfo>(ArrayMemberFunctionInfo::Operation::Destroy, this));
	memberFunctions.emplace("size", std::make_unique<ArrayMemberFunctionInfo>(ArrayMemberFunctionInfo::Operation::Size, this));
	memberFunctions.emplace("[]", std::make_unique<ArrayMemberFunctionInfo>(ArrayMemberFunctionInfo::Operation::Index, this));
	arrayItemType.addDependentType(this);
}


ArrayTypeInfo::~ArrayTypeInfo()
{
}


std::any ArrayTypeInfo::index(Array* array, int index)
{
	if (array != nullptr && index >= 0 && index < array->size)
	{
		std::size_t itemSize = arrayItemType.getTypeSize();
		uintptr_t itemAddress = reinterpret_cast<uintptr_t>(array->arrayData + index * itemSize);
		return arrayItemType.toPointer().createAnyOfType(itemAddress);
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
		LLVM::LLVMCatIntrinsics::freeMemory(array->arrayData);
		if constexpr (Configuration::logJitCatObjectConstructionEvents)
		{
			std::cout << "(ArrayTypeInfo::placementDestruct) deallocated buffer of size " << std::dec << array->size * typeSize << ": " << std::hex << reinterpret_cast<uintptr_t>(array->arrayData) << "\n";
		}
		array->arrayData = nullptr;
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
	target->arrayData = new unsigned char[target->size * valueSize];

	if constexpr (Configuration::logJitCatObjectConstructionEvents)
	{
		std::cout << "(ArrayManipulator::copyConstruct) Allocated buffer of size " << std::dec << target->size * valueSize << ": " << std::hex << reinterpret_cast<uintptr_t>(target->arrayData) << "\n";
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
	target->arrayData = source->arrayData;
	source->size = 0;
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


ArrayTypeInfo& ArrayTypeInfo::createArrayTypeOf(const CatGenericType& arrayItemType)
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


void ArrayTypeInfo::deleteArrayTypeOfType(const CatGenericType& arrayItemType)
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


std::size_t ArrayTypeInfo::getItemSize() const
{
	return arrayItemType.getTypeSize();
}


std::vector<ArrayTypeInfo*> ArrayTypeInfo::arrayTypes = std::vector<ArrayTypeInfo*>();