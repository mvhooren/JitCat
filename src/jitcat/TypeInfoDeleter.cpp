#include "jitcat/TypeInfoDeleter.h"
#include "jitcat/TypeInfo.h"


void jitcat::Reflection::TypeInfoDeleter::operator()(TypeInfo* typeInfo) const
{
	TypeInfo::destroy(typeInfo);
}
