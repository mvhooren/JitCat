/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/CatError.h"

using namespace jitcat;


CatError::CatError()
{
}


CatError::CatError(const CatError& other):
	message(other.message)
{
}


CatError::CatError(const std::string& message): 
	message(message)
{
}


CatError::CatError(const char* message):
	message(message)
{
}


CatError& CatError::operator=(const CatError& other)
{
	message = other.message;
	return *this;
}


bool CatError::operator==(const CatError& other) const
{
	return message == other.message;
}


const std::string& CatError::getMessage() const
{
	return message;
}


CatError CatError::defaultError = CatError();
