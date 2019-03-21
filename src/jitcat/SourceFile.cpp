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
	setSource(fileContents, context);
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
	if (context == nullptr)
	{
		context = &CatRuntimeContext::defaultContext;
		context->getErrorManager()->clear();
	}
	ExpressionErrorManager* errorManager = nullptr;
	errorManager = context->getErrorManager();
	errorManagerHandle = errorManager;
	parseResult.reset(JitCat::get()->parseFull(sourceText.get(), context, errorManager, this));
	if (parseResult->success)
	{
		AST::CatSourceFile* sourceFileNode = parseResult->getNode<AST::CatSourceFile>();
		sourceFileNode->print();
		if (errorManager != nullptr)
		{
			errorManager->compiledWithoutErrors(this);
		}
	}
}

void jitcat::SourceFile::setSource(const std::string& source, CatRuntimeContext* context)
{
	if (context == nullptr)
	{
		context = &CatRuntimeContext::defaultContext;
		context->getErrorManager()->clear();
	}
	context->getErrorManager()->errorSourceDeleted(this);	
	sourceText.reset(new Document(source.c_str(), source.size()));
	compile(context);
}
