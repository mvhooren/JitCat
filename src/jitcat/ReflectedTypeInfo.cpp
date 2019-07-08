#include "jitcat/ReflectedTypeInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


jitcat::Reflection::ReflectedTypeInfo::ReflectedTypeInfo(const char* typeName, std::size_t typeSize, TypeCaster* typeCaster, bool allowConstruction, 
														std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor, 
														std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& copyConstructor,
														std::function<void(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)>& moveConstructor,
														std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor):
	TypeInfo(typeName, typeSize, typeCaster),
	placementConstructor(placementConstructor),
	copyConstructor(copyConstructor),
	moveConstructor(moveConstructor),
	placementDestructor(placementDestructor),
	allowConstruction(allowConstruction),
	allowInheritance(allowConstruction),
	allowCopyConstruction(false),
	allowMoveConstruction(false),
	triviallyCopyable(false),
	inheritanceCheckFunction([](CatRuntimeContext*, AST::CatClassDefinition*, ExpressionErrorManager*, void*){return true;})
{
}


jitcat::Reflection::ReflectedTypeInfo::~ReflectedTypeInfo()
{
	ArrayManipulator::deleteArrayManipulatorsOfType(this);
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::enableConstruction()
{
	allowConstruction = true;
	return *this;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::disableConstruction()
{
	allowConstruction = false;
	allowCopyConstruction = false;
	allowMoveConstruction = false;
	allowInheritance = false;
	return *this;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::enableCopyConstruction()
{
	allowCopyConstruction = true;
	return *this;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::disableCopyConstruction()
{
	allowCopyConstruction = false;
	return *this;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::enableMoveConstruction()
{
	allowMoveConstruction = true;
	return *this;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::disableMoveConstruction()
{
	allowMoveConstruction = false;
	return *this;
}


void jitcat::Reflection::ReflectedTypeInfo::setTriviallyCopyable(bool triviallyCopyable_)
{
	triviallyCopyable = triviallyCopyable_;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::enableInheritance()
{
	allowInheritance = true;
	return *this;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::disableInheritance()
{
	allowInheritance = false;
	return *this;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::setInheritanceChecker(std::function<bool(CatRuntimeContext*, AST::CatClassDefinition*, ExpressionErrorManager*, void*)>& checkFunction)
{
	inheritanceCheckFunction = checkFunction;
	return *this;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::setConstructors(std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor_, 
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


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::setTypeSize(std::size_t newSize)
{
	typeSize = newSize;
	return *this;
}


void jitcat::Reflection::ReflectedTypeInfo::placementConstruct(unsigned char* buffer, std::size_t bufferSize) const
{
	assert(allowConstruction);
	placementConstructor(buffer, bufferSize);
}


void jitcat::Reflection::ReflectedTypeInfo::placementDestruct(unsigned char* buffer, std::size_t bufferSize)
{
	assert(allowConstruction);
	placementDestructor(buffer, bufferSize);
}


void jitcat::Reflection::ReflectedTypeInfo::copyConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, const unsigned char* sourceBuffer, std::size_t sourceBufferSize)
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


void jitcat::Reflection::ReflectedTypeInfo::moveConstruct(unsigned char* targetBuffer, std::size_t targetBufferSize, unsigned char* sourceBuffer, std::size_t sourceBufferSize)
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


bool jitcat::Reflection::ReflectedTypeInfo::getAllowInheritance() const
{
	return allowInheritance;
}


bool jitcat::Reflection::ReflectedTypeInfo::inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext)
{
	return inheritanceCheckFunction(context, childClass, errorManager, errorContext);
}


bool jitcat::Reflection::ReflectedTypeInfo::getAllowConstruction() const
{
	return allowConstruction;
}


bool jitcat::Reflection::ReflectedTypeInfo::getAllowCopyConstruction() const
{
	return allowCopyConstruction;
}


bool jitcat::Reflection::ReflectedTypeInfo::getAllowMoveConstruction() const
{
	return allowMoveConstruction;
}


bool jitcat::Reflection::ReflectedTypeInfo::isTriviallyCopyable() const
{
	return triviallyCopyable;
}
