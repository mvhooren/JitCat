/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ProductionTokenSetMember.h"
#include "ProductionToken.h"


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