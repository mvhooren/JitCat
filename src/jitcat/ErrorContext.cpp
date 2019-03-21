/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ErrorContext.h"
#include "jitcat/CatRuntimeContext.h"

using namespace jitcat;


ErrorContext::ErrorContext(CatRuntimeContext* context, const std::string& contextDescription):
	context(context),
	contextDescription(contextDescription)
{
	context->pushErrorContext(this);
}


ErrorContext::~ErrorContext()
{
	context->popErrorContext(this);
}


const std::string& ErrorContext::getContextDescription() const
{
	return contextDescription;
}
