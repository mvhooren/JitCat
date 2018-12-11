#include "Timer.h"


Timer::Timer()
{
	getTimeSincePreviousCall();
}


inline double Timer::getTimeSincePreviousCall()
{
	std::chrono::high_resolution_clock::time_point newTime = Clock::now();
	long long diff = std::chrono::duration_cast<std::chrono::nanoseconds>(newTime - timer).count();
	timer = newTime;
	return (double)diff / 1000000000.0;
}
