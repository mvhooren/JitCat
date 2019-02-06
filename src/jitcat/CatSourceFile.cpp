#include "jitcat/CatSourceFile.h"
#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatDefinition.h"
#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatLog.h"

#include <cassert>

using namespace jitcat;
using namespace jitcat::AST;
using namespace jitcat::Tools;


jitcat::AST::CatSourceFile::CatSourceFile(const std::string& name, std::vector<std::unique_ptr<CatDefinition>>&& definitions):
	name(name),
	definitions(std::move(definitions))
{
	for (auto& iter : this->definitions)
	{
		switch (iter->getNodeType())
		{
			case CatASTNodeType::ClassDefinition:		classDefinitions.push_back(static_cast<CatClassDefinition*>(iter.get())); break;
			case CatASTNodeType::FunctionDefinition:	functionDefinitions.push_back(static_cast<CatFunctionDefinition*>(iter.get())); break;
			default:
				assert(false);

		}

	}
}


jitcat::AST::CatSourceFile::~CatSourceFile()
{
}


void jitcat::AST::CatSourceFile::print() const
{
	for (auto& iter : definitions)
	{
		iter->print();
		CatLog::log("\n\n");
	}
}


CatASTNodeType jitcat::AST::CatSourceFile::getNodeType()
{
	return CatASTNodeType::SourceFile;
}


const std::vector<const CatClassDefinition*>& jitcat::AST::CatSourceFile::getClassDefinitions() const
{
	return classDefinitions;
}


const std::vector<const CatFunctionDefinition*>& jitcat::AST::CatSourceFile::getFunctionDefinitions() const
{
	return functionDefinitions;
}
