#include "jitcat/ReflectedTypeInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


jitcat::Reflection::ReflectedTypeInfo::ReflectedTypeInfo(const char* typeName, std::size_t typeSize, TypeCaster* typeCaster, bool allowConstruction, 
														std::function<Reflectable* ()>& constructor, 
														std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor, 
														std::function<void(Reflectable*)>& destructor, 
														std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor):
	TypeInfo(typeName, typeSize, typeCaster),
	constructor(constructor),
	placementConstructor(placementConstructor),
	destructor(destructor),
	placementDestructor(placementDestructor),
	allowConstruction(allowConstruction),
	allowInheritance(allowConstruction),
	inheritanceCheckFunction([](CatRuntimeContext*, AST::CatClassDefinition*, ExpressionErrorManager*, void*){return true;})
{
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::enableConstruction()
{
	allowConstruction = true;
	return *this;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::disableConstruction()
{
	allowConstruction = false;
	allowInheritance = false;
	return *this;
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


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::setConstructors(std::function<Reflectable* ()>& constructor_, 
																		  std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementConstructor_, 
																		  std::function<void(Reflectable*)>& destructor_, 
																		  std::function<void(unsigned char* buffer, std::size_t bufferSize)>& placementDestructor_)
{
	constructor = constructor_;
	placementConstructor = placementConstructor_;
	destructor = destructor_;
	placementDestructor = placementDestructor_;
	return *this;
}


ReflectedTypeInfo& jitcat::Reflection::ReflectedTypeInfo::setTypeSize(std::size_t newSize)
{
	typeSize = newSize;
	return *this;
}


void jitcat::Reflection::ReflectedTypeInfo::construct(unsigned char* buffer, std::size_t bufferSize) const
{
	placementConstructor(buffer, bufferSize);
}


Reflectable* jitcat::Reflection::ReflectedTypeInfo::construct() const
{
	return constructor();
}


void jitcat::Reflection::ReflectedTypeInfo::destruct(Reflectable* object)
{
	destructor(object);
}


void jitcat::Reflection::ReflectedTypeInfo::destruct(unsigned char* buffer, std::size_t bufferSize)
{
	placementDestructor(buffer, bufferSize);
}


bool jitcat::Reflection::ReflectedTypeInfo::getAllowInheritance() const
{
	return allowInheritance;
}


bool jitcat::Reflection::ReflectedTypeInfo::inheritTypeCheck(CatRuntimeContext* context, AST::CatClassDefinition* childClass, ExpressionErrorManager* errorManager, void* errorContext)
{
	return true;
}


bool jitcat::Reflection::ReflectedTypeInfo::getAllowConstruction() const
{
	return allowConstruction;
}


bool jitcat::Reflection::ReflectedTypeInfo::isTriviallyCopyable() const
{
	return false;
}
