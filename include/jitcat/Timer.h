/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#pragma once

#include <chrono>

namespace jitcat::Tools
{

	class Timer
	{
		typedef std::chrono::high_resolution_clock Clock;
		std::chrono::high_resolution_clock::time_point timer;
	public:
		Timer();

		double getTimeSincePreviousCall();
	};

} //End namespace jitcat::Tools