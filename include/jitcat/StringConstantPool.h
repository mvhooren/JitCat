/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2020
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/


#pragma once


#include "jitcat/Configuration.h"

#include <unordered_set>


namespace jitcat::AST
{
	class StringConstantPool
	{
		StringConstantPool() = delete;
		~StringConstantPool() = delete;
		StringConstantPool(const StringConstantPool&) = delete;
		StringConstantPool& operator=(const StringConstantPool&) = delete;
	public:
		static const Configuration::CatString* getString(const Configuration::CatString& string);

		//Warning: This will clear all the strings in the pool, including those that are still being referenced by any code.
		static void clearPool();

	private:
		static std::unordered_set<Configuration::CatString> pool;
	};
}