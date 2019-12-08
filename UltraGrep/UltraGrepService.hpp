#pragma once
#include <string>
#include <queue>


struct UltraGrepService {
	using StringQueue = std::queue<std::string>;
	static bool runUltraGrep(StringQueue& arguments, std::mutex* p_mxOutput, StringQueue& output);
};