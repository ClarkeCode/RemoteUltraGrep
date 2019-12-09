/**
@file:		PerformanceTimer.cpp
@author:	Robert Clarke
@date:		2019-11-08
@note:		Developed for the INFO-5104 course at Fanshawe College
@brief:		Declaration of the PerformanceTimer class methods
*/
#include "PerformanceTimer.hpp"

//timer::PerformanceTimer::PerformanceTimer() :
//	liStartTime(LARGE_INTEGER()), liEndTime(LARGE_INTEGER()), liTicksPerSecond(LARGE_INTEGER()), hasStopped(false) {
//	QueryPerformanceCounter(&liStartTime);
//	QueryPerformanceFrequency(&liTicksPerSecond);
//}
//
//void timer::PerformanceTimer::stop() {
//	hasStopped = true;
//	QueryPerformanceCounter(&liEndTime);
//}
//
//double timer::PerformanceTimer::getTime() {
//	if (!hasStopped)
//		stop();
//	return (liEndTime.QuadPart - liStartTime.QuadPart) / double(liTicksPerSecond.QuadPart);
//}