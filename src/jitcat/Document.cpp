/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Document.h"

#include <string.h>

using namespace jitcat::Tokenizer;


Document::Document(const char* fileData, std::size_t fileSize):
	size(fileSize)
{
	data.reset(new char[fileSize + 1]);
	memcpy(data.get(), fileData, fileSize);
	data.get()[fileSize] = 0;
}


Document::~Document()
{
}


const char* Document::getDocumentData() const
{
	return data.get();
}


std::size_t Document::getDocumentSize() const
{
	return size;
}