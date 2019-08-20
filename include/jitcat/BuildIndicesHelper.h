/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2019
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <cstddef>

namespace jitcat
{
	////Indices trick
	//https://stackoverflow.com/questions/15014096/c-index-of-type-during-variadic-template-expansion
	//Allows getting indices per template in variadic template expansion
	template <std::size_t... Is>
	struct Indices {};
 
	template <std::size_t N, std::size_t... Is>
	struct BuildIndices
		: BuildIndices<N-1, N-1, Is...> {};
 
	template <std::size_t... Is>
	struct BuildIndices<0, Is...> : Indices<Is...> {};
}