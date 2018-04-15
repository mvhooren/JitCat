/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "ParseHelper.h"


bool ParseHelper::isValidStringChar(const char symbol) 
{
	return (symbol >= ' ' || symbol == '\t') && (unsigned char)symbol != 0x7F && (unsigned char)symbol != 0xFF;
}


bool ParseHelper::isAlphaNumeric(const char symbol) 
{
	return    (symbol >= 'a' && symbol <= 'z') 
		   || (symbol >= 'A' && symbol <= 'Z');
}


bool ParseHelper::isNumber(const char symbol) 
{
	return symbol >= '0' && symbol <= '9';
}


bool ParseHelper::isNonZeroNumber(const char symbol) 
{
	return symbol >= '1' && symbol <= '9';
}


bool ParseHelper::isOctNumber(const char symbol) 
{
	return symbol >= '0' && symbol <= '7';
}


bool ParseHelper::isHexDigit(const char symbol) 
{
	return    (symbol >= '0' && symbol <= '9')
		   || (symbol >= 'a' && symbol <= 'f')
		   || (symbol >= 'A' && symbol <= 'F');
}

