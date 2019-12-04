/**
@file:		PerformanceTimer.hpp
@author:	Robert Clarke
@date:		2019-11-08
@note:		Developed for the INFO-5104 course at Fanshawe College
@brief:		Declaration of the PerformanceTimer class
*/
#pragma once
namespace timer {
#include <Windows.h>
	class PerformanceTimer {
		LARGE_INTEGER liStartTime, liEndTime, liTicksPerSecond;
		bool hasStopped;
	public:
		PerformanceTimer();
		void stop();
		double getTime();
	};
}
