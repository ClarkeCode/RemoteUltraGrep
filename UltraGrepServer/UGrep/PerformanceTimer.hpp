/**
@file:		PerformanceTimer.hpp
@author:	Robert Clarke
@date:		2019-11-08
@note:		Developed for the INFO-5104 course at Fanshawe College
@brief:		Declaration of the PerformanceTimer class
*/
#pragma once
#include <chrono>
//#include <Windows.h>
namespace timer {
	class PerformanceTimer {
		//LARGE_INTEGER liStartTime, liEndTime, liTicksPerSecond;
		std::chrono::steady_clock::time_point startTime, endTime;
		bool hasStopped;
	public:
		PerformanceTimer() : hasStopped(false), 
			startTime(std::chrono::high_resolution_clock::now()), endTime(std::chrono::high_resolution_clock::now()) {};
		inline void stop() {
			hasStopped = true;
			endTime = std::chrono::high_resolution_clock::now();
		};
		double getTime() {
			return (double)std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
		};
	};
}
