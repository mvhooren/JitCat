#include "jitcat/StaticConstMemberInfo.h"
#include "jitcat/Tools.h"


using namespace jitcat;
using namespace jitcat::Reflection;


StaticConstMemberInfo::StaticConstMemberInfo(const std::string& name, const CatGenericType& type, const std::any& value):
	name(name),
	lowerCaseName(Tools::toLowerCase(name)),
	type(type),
	value(value)
{
}


const std::string& StaticConstMemberInfo::getName() const
{
	return name;
}


const std::string& StaticConstMemberInfo::getLowerCaseName() const
{
	return lowerCaseName;
}


const CatGenericType& StaticConstMemberInfo::getType() const
{
	return type;
}


const std::any& StaticConstMemberInfo::getValue() const
{
	return value;
}
