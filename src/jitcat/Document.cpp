/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "Document.h"

#include <string.h>


Document::Document(const char* fileData, std::size_t fileSize):
	size(fileSize)
{
	data = new char[fileSize + 1];
	memcpy(data, fileData, fileSize);
	data[fileSize] = 0;
}


Document::~Document()
{
	delete[] data;
}


const char* Document::getDocumentData() const
{
	return data;
}


std::size_t Document::getDocumentSize() const
{
	return size;
}