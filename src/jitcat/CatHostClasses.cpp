#include "jitcat/CatHostClasses.h"
#include "jitcat/Tools.h"
#include "jitcat/TypeRegistry.h"


using namespace jitcat;


CatHostClasses::CatHostClasses()
{
}


CatHostClasses::~CatHostClasses()
{
}


void jitcat::CatHostClasses::addHostClass(std::unique_ptr<CatHostClass>& hostClass, const std::string& className)
{
	std::string lowerCaseTypeName = Tools::toLowerCase(className);
	if (hostClasses.find(lowerCaseTypeName) == hostClasses.end())
	{
		hostClasses.emplace(lowerCaseTypeName, hostClass.release());
	}
}


CatHostClass* jitcat::CatHostClasses::getHostClass(const std::string& hostClassName) const
{
	std::string lowerCaseTypeName = Tools::toLowerCase(hostClassName);
	auto iter = hostClasses.find(lowerCaseTypeName);
	if (iter != hostClasses.end())
	{
		return iter->second.get();
	}
	return nullptr;
}
