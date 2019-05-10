#include "jitcat/CatHostClasses.h"
#include "jitcat/Tools.h"


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
