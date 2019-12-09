#pragma once
#include "CommandLineFunctions.hpp"
#include "FileReporting.hpp"
#include "ThreadPool.hpp"

#include <string>
#include <queue>
#include <mutex>

namespace UltraGrepService {
	using StringQueue = std::queue<std::string>;
	bool runUltraGrep(StringQueue& arguments, std::mutex* p_mxOutput, StringQueue& output);
}
//struct UltraGrepService {
//	using StringQueue = std::queue<std::string>;
//	UltraGrepService() = default;
//	~UltraGrepService() = default;
//};

