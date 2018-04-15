/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ErrorContext.h"
#include "CatRuntimeContext.h"


ErrorContext::ErrorContext(CatRuntimeContext* context, const std::string& contextDescription):
	context(context),
	contextDescription(contextDescription)
{
	if (context != nullptr)
	{
		context->pushErrorContext(this);
	}
}


ErrorContext::~ErrorContext()
{
	if (context != nullptr)
	{
		context->popErrorContext(this);
	}
}


const std::string& ErrorContext::getContextDescription() const
{
	return contextDescription;
}
