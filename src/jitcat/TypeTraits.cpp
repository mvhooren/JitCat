/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#include "jitcat/TypeTraits.h"
using namespace jitcat;

const TypeID jitcat::TypeCounter::getNextTypeId()
{
	return typeIdCounter++;
}


std::atomic<TypeID> jitcat::TypeCounter::typeIdCounter = 1;