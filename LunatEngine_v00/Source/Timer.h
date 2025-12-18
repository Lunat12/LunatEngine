#pragma once
#include <chrono>
#include <iostream>

class Timer
{
public:
	
	void begin();
	

	void stop();
	
	
	long ReadFPS();
	

private:
	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point finish;
	std::chrono::milliseconds milliseconds;
	//bool isRunning;
	//bool is running FALTA.
};

