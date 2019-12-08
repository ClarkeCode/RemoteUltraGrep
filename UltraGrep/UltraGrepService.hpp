#pragma once
#include <string>
#include <queue>
#include <mutex>


struct UltraGrepService {
	using StringQueue = std::queue<std::string>;
	static bool runUltraGrep(StringQueue& arguments, std::mutex* p_mxOutput, StringQueue& output);
};