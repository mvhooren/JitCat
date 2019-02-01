/*
  This file is part of the JitCat library.
	
  Copyright (C) Machiel van Hooren 2018
  Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
*/

#include "jitcat/Timer.h"

using namespace jitcat::Tools;


Timer::Timer()
{
	getTimeSincePreviousCall();
}


double Timer::getTimeSincePreviousCall()
{
	std::chrono::high_resolution_clock::time_point newTime = Clock::now();
	long long diff = std::chrono::duration_cast<std::chrono::nanoseconds>(newTime - timer).count();
	timer = newTime;
	return (double)diff / 1000000000.0;
}
