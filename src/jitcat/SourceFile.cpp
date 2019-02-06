#include "jitcat/SourceFile.h"
#include "jitcat/CatClassDefinition.h"
#include "jitcat/CatFunctionDefinition.h"
#include "jitcat/CatRuntimeContext.h"
#include "jitcat/CatSourceFile.h"
#include "jitcat/Document.h"
#include "jitcat/ExpressionErrorManager.h"
#include "jitcat/JitCat.h"
#include "jitcat/Reflectable.h"
#include "jitcat/SLRParseResult.h"


using namespace jitcat;
using namespace jitcat::Parser;
using namespace jitcat::Tokenizer;


SourceFile::SourceFile(const std::string& fileContents, CatRuntimeContext* context)
{
	sourceText.reset(new Document(fileContents.c_str(), fileContents.size()));
	if (context != nullptr)
	{
		compile(context);
	}
}


SourceFile::~SourceFile()
{
	if (errorManagerHandle.getIsValid())
	{
		static_cast<ExpressionErrorManager*>(errorManagerHandle.get())->errorSourceDeleted(this);
	}
}


void SourceFile::compile(CatRuntimeContext* context)
{
	if (context != nullptr)
	{
		errorManagerHandle = context->getErrorManager();
	}
	else
	{
		errorManagerHandle = nullptr;
	}
	parseResult.reset(JitCat::get()->parseFull(sourceText.get(), context));
	if (parseResult->success)
	{
		AST::CatSourceFile* sourceFileNode = parseResult->getNode<AST::CatSourceFile>();
		sourceFileNode->print();
	}
}