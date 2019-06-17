#include "jitcat/ReflectedTypeInfo.h"

using namespace jitcat;
using namespace jitcat::Reflection;


inline void jitcat::Reflection::ReflectedTypeInfo::construct(unsigned char* buffer, std::size_t bufferSize) const
{
	placementConstructor(buffer, bufferSize);
}


inline Reflectable* jitcat::Reflection::ReflectedTypeInfo::construct() const
{
	return constructor();
}


inline void jitcat::Reflection::ReflectedTypeInfo::destruct(Reflectable* object)
{
	destructor(object);
}
