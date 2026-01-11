#include "Globals.h"
#include "Timer.h"




void Timer::begin()
{
	start = { std::chrono::steady_clock::now() };
	//isRunning = true;
}

void Timer::stop()
{
	finish = { std::chrono::steady_clock::now() };
	milliseconds= std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
	
	//isRunning = false;
}

long Timer::ReadFPS()
{
	return milliseconds.count();
}