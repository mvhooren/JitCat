#include "jitcat/VectorTypeInfo.h"
#include "jitcat/LLVMCatIntrinsics.h"
#include "jitcat/TypeCaster.h"
#include "jitcat/Tools.h"
#include "jitcat/VectorMemberFunctionInfo.h"

#include <cassert>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <string>


using namespace jitcat;
using namespace jitcat::Reflection;


VectorTypeInfo::VectorTypeInfo(CatGenericType scalarType, unsigned int length,
							   std::unique_ptr<TypeCaster>&& typeCaster):
	TypeInfo(getVectorTypeName(scalarType, length), scalarType.getTypeSize() * length, std::move(typeCaster)),
	scalarType(scalarType),
	length(length)
{
	memberFunctions.emplace("__init", std::make_unique<VectorMemberFunctionInfo>(VectorMemberFunctionInfo::Operation::Init, this));
	memberFunctions.emplace("__destroy", std::make_unique<VectorMemberFunctionInfo>(VectorMemberFunctionInfo::Operation::Destroy, this));
	memberFunctions.emplace("[]", std::make_unique<VectorMemberFunctionInfo>(VectorMemberFunctionInfo::Operation::Index, this));
}


VectorTypeInfo::~VectorTypeInfo()
{
}


const CatGenericType& VectorTypeInfo::getScalarType() const
{
	return scalarType;
}


std::size_t VectorTypeInfo::getScalarSize() const
{
	return scalarType.getTypeSize();
}


std::size_t VectorTypeInfo::getSize() const
{
	return getScalarSize() * length;
}


std::size_t jitcat::Reflection::VectorTypeInfo::getLength() const
{
	return length;
}


bool VectorTypeInfo::isVectorType() const
{
	return true;
}


void VectorTypeInfo::placementConstruct(unsigned char* buffer, std::size_t bufferSize) const
{
	assert(bufferSize >= getSize());
	memset(buffer, 0, getSize());
}


void VectorTypeInfo::placementDestruct(unsigned char* buffer, std::size_t bufferSize)
{
}


void VectorTypeInfo::copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	assert(targetBufferSize >= getSize());
	assert(sourceBufferSize >= getSize());
	assert(targetBuffer != sourceBuffer);
	memcpy(targetBuffer, sourceBuffer, getSize());
}


void VectorTypeInfo::moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	copyConstruct(targetBuffer, targetBufferSize, sourceBuffer, sourceBufferSize);
}


bool VectorTypeInfo::isTriviallyConstructable() const
{
	return true;
}


bool VectorTypeInfo::getAllowInheritance() const
{
	return false;
}


bool VectorTypeInfo::inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext)
{
	return false;
}


bool VectorTypeInfo::getAllowConstruction() const
{
	return true;
}


bool VectorTypeInfo::getAllowCopyConstruction() const
{
	return true;
}

bool VectorTypeInfo::getAllowMoveConstruction() const
{
	return true;
}


const char* VectorTypeInfo::getVectorTypeName(const CatGenericType& scalarType, unsigned int length)
{
	static std::unordered_set<std::string> names = std::unordered_set<std::string>();
	std::string postfix = "";
	if (scalarType.isDoubleType())
		postfix = "d";
	else if (scalarType.isIntType())
		postfix = "i";
	else if (scalarType.isInt64Type())
		postfix = "l";
	else if (scalarType.isUIntType())
		postfix = "u";
	else if (scalarType.isUInt64Type())
		postfix = "ul";
	else if (!scalarType.isFloatType())
		assert(false);
	std::string name = Tools::append("vector", length, postfix);
	if (auto iter = names.find(name);
		iter == names.end())
	{
		names.insert(name);
	}
	return (*names.find(name)).c_str();
}


std::vector<VectorTypeInfo*> VectorTypeInfo::vectorTypes = std::vector<VectorTypeInfo*>();