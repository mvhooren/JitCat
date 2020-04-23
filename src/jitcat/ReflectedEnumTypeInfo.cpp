/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ReflectedEnumTypeInfo.h"
#include "jitcat/StaticConstMemberInfo.h"
#include "jitcat/TypeCaster.h"


#include <cassert>

using namespace jitcat;
using namespace jitcat::Reflection;


ReflectedEnumTypeInfo::ReflectedEnumTypeInfo(const char* typeName, const CatGenericType& underlyingType, std::size_t typeSize, std::unique_ptr<TypeCaster> caster):
    TypeInfo(typeName, typeSize, std::move(caster)),
    underlyingType(underlyingType),
    defaultValueBuffer(nullptr),
    defaultValueSize(0)
{
}


ReflectedEnumTypeInfo::~ReflectedEnumTypeInfo()
{
}


void ReflectedEnumTypeInfo::placementConstruct(unsigned char* buffer, std::size_t bufferSize) const
{
    if (underlyingType.isTriviallyCopyable())
    {
        assert(bufferSize >= defaultValueSize);
        memcpy(buffer, defaultValueBuffer, defaultValueSize);
    }
    else
    {
        underlyingType.copyConstruct(buffer, bufferSize, defaultValueBuffer, defaultValueSize);
    }
}


void ReflectedEnumTypeInfo::placementDestruct(unsigned char* buffer, std::size_t bufferSize)
{
    underlyingType.placementDestruct(buffer, bufferSize);
}


void ReflectedEnumTypeInfo::copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
    underlyingType.copyConstruct(targetBuffer, targetBufferSize, sourceBuffer, sourceBufferSize);
}


void ReflectedEnumTypeInfo::moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
    underlyingType.moveConstruct(targetBuffer, targetBufferSize, sourceBuffer, sourceBufferSize);
}


bool ReflectedEnumTypeInfo::getAllowInheritance() const
{
    return false;
}


bool ReflectedEnumTypeInfo::inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext)
{
    return false;
}


bool ReflectedEnumTypeInfo::getAllowConstruction() const
{
    return defaultUnderlyingValue.has_value() && underlyingType.isConstructible();
}


bool ReflectedEnumTypeInfo::getAllowCopyConstruction() const
{
    return underlyingType.isCopyConstructible();
}


bool ReflectedEnumTypeInfo::getAllowMoveConstruction() const
{
    return underlyingType.isMoveConstructible();
}


bool ReflectedEnumTypeInfo::isTriviallyCopyable() const
{
    return underlyingType.isTriviallyCopyable();
}
