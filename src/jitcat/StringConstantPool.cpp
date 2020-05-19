/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/StringConstantPool.h"

using namespace jitcat;
using namespace jitcat::AST;


const Configuration::CatString* StringConstantPool::getString(const Configuration::CatString& string)
{
    const auto& poolItem = pool.find(string);
    if (poolItem == pool.end())
    {
        pool.insert(string);
        return  &(*pool.find(string));
    }
    return &(*poolItem);
}


void StringConstantPool::clearPool()
{
    pool.clear();
}


std::unordered_set<Configuration::CatString> StringConstantPool::pool = std::unordered_set<Configuration::CatString>();