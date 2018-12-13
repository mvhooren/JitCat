#pragma once

#include <chrono>


class Timer
{
	typedef std::chrono::high_resolution_clock Clock;
	std::chrono::high_resolution_clock::time_point timer;
public:
	Timer();

	double getTimeSincePreviousCall();
};