/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "CatError.h"


CatError::CatError()
{
}


CatError::CatError(const std::string& message): 
	message(message)
{
}


CatError CatError::defaultError = CatError();
