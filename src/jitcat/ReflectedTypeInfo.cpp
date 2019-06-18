#include "jitcat/ReflectedTypeInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


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
