/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/ProductionTokenSetMember.h"
#include "jitcat/ProductionToken.h"

using namespace jitcat::Grammar;


bool ProductionTokenSetMember::operator== (const ProductionToken& other) const
{
	if (other.getType() == getType())
	{
		return equals(other);
	}
	else
	{
		return false;
	}
}