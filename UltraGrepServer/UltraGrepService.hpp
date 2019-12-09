/*
Created by Robert Clarke and Evan Burgess
Date: 2019-12-08
*/
#pragma once
#include "UGrep/CommandLineFunctions.hpp"
#include "UGrep/FileReporting.hpp"
#include "UGrep/ThreadPool.hpp"

#include <string>
#include <queue>
#include <mutex>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <mutex>
#include <functional>

bool runUltraGrep(std::queue<std::string>& arguments, std::mutex* p_mxOutput, std::queue<std::string>& output);