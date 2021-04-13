/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ReflectedTypeInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


jitcat::Reflection::ReflectedTypeInfo::ReflectedTypeInfo(const char* typeName, std::size_t typeSize, std::unique_ptr<TypeCaster> typeCaster, bool allowConstruction, 
									 std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor, 
									 std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& copyConstructor,
									 std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& moveConstructor,
									 std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor):
	TypeInfo(typeName, typeSize, std::move(typeCaster)),
	placementConstructor(placementConstructor),
	copyConstructor(copyConstructor),
	moveConstructor(moveConstructor),
	placementDestructor(placementDestructor),
	inheritanceCheckFunction([](CatRuntimeContext*, AST::CatClassDefinition*, ExpressionErrorManager*, void*){return true;}),
	allowConstruction(allowConstruction),
	allowCopyConstruction(false),
	allowMoveConstruction(false),
	allowInheritance(allowConstruction),
	triviallyCopyable(false)
{
}


ReflectedTypeInfo::~ReflectedTypeInfo()
{
}


ReflectedTypeInfo& ReflectedTypeInfo::enableConstruction()
{
	allowConstruction = true;
	return *this;
}


ReflectedTypeInfo& ReflectedTypeInfo::disableConstruction()
{
	allowConstruction = false;
	allowCopyConstruction = false;
	allowMoveConstruction = false;
	allowInheritance = false;
	return *this;
}


ReflectedTypeInfo& ReflectedTypeInfo::enableCopyConstruction()
{
	allowCopyConstruction = true;
	return *this;
}


ReflectedTypeInfo& ReflectedTypeInfo::disableCopyConstruction()
{
	allowCopyConstruction = false;
	return *this;
}


ReflectedTypeInfo& ReflectedTypeInfo::enableMoveConstruction()
{
	allowMoveConstruction = true;
	return *this;
}


ReflectedTypeInfo& ReflectedTypeInfo::disableMoveConstruction()
{
	allowMoveConstruction = false;
	return *this;
}


void ReflectedTypeInfo::setTriviallyCopyable(bool triviallyCopyable_)
{
	triviallyCopyable = triviallyCopyable_;
}


ReflectedTypeInfo& ReflectedTypeInfo::enableInheritance()
{
	allowInheritance = true;
	return *this;
}


ReflectedTypeInfo& ReflectedTypeInfo::disableInheritance()
{
	allowInheritance = false;
	return *this;
}


ReflectedTypeInfo& ReflectedTypeInfo::setInheritanceChecker(const std::function<bool(CatRuntimeContext*, AST::CatClassDefinition*, ExpressionErrorManager*, void*)>& checkFunction)
{
	inheritanceCheckFunction = checkFunction;
	return *this;
}


ReflectedTypeInfo& ReflectedTypeInfo::setConstructors(std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor_, 
																		  std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& copyConstructor_,
																		  std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& moveConstructor_,
																		  std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor_)
{
	placementConstructor = placementConstructor_;
	copyConstructor = copyConstructor_;
	moveConstructor = moveConstructor_;
	placementDestructor = placementDestructor_;
	return *this;
}


ReflectedTypeInfo& ReflectedTypeInfo::setTypeSize(std::size_t newSize)
{
	typeSize = newSize;
	return *this;
}


bool ReflectedTypeInfo::isReflectedType() const
{
	return true;
}


void ReflectedTypeInfo::placementConstruct(unsigned char* buffer, std::size_t bufferSize) const
{
	assert(allowConstruction);
	placementConstructor(buffer, bufferSize);
}


void ReflectedTypeInfo::placementDestruct(unsigned char* buffer, std::size_t bufferSize)
{
	assert(allowConstruction);
	placementDestructor(buffer, bufferSize);
}


void ReflectedTypeInfo::copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	assert(allowCopyConstruction);
	assert(sourceBufferSize >= targetBufferSize);
	if (triviallyCopyable)
	{
		memcpy(targetBuffer, sourceBuffer, targetBufferSize);
	}
	else
	{
		copyConstructor(targetBuffer, targetBufferSize, sourceBuffer, sourceBufferSize);
	}
}


void ReflectedTypeInfo::moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
{
	assert(allowMoveConstruction);
	assert(sourceBufferSize >= targetBufferSize);
	if (triviallyCopyable)
	{
		memcpy(targetBuffer, sourceBuffer, targetBufferSize);
	}
	else
	{
		moveConstructor(targetBuffer, targetBufferSize, sourceBuffer, sourceBufferSize);
	}
}


bool ReflectedTypeInfo::getAllowInheritance() const
{
	return allowInheritance;
}


bool ReflectedTypeInfo::inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext)
{
	return inheritanceCheckFunction(context, childClass, errorManager, errorContext);
}


bool ReflectedTypeInfo::getAllowConstruction() const
{
	return allowConstruction;
}


bool ReflectedTypeInfo::getAllowCopyConstruction() const
{
	return allowCopyConstruction;
}


bool ReflectedTypeInfo::getAllowMoveConstruction() const
{
	return allowMoveConstruction;
}


bool ReflectedTypeInfo::isTriviallyCopyable() const
{
	return triviallyCopyable;
}
