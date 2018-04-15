/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "StackItemProduction.h"

StackItemProduction::~StackItemProduction()
{
	for (int i = 0; i < (int)children.size(); i++)
	{
		delete children[i];
	}
}