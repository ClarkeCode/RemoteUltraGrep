/**
@file:		CommandLineFunctions.hpp
@author:	Robert Clarke
@date:		2019-11-08
@note:		Developed for the INFO-5104 course at Fanshawe College
@brief:		Declaration of the unpackCommandLineArgs function
*/

#pragma once
#include <queue>
#include <string>
#include <filesystem>
#include <regex>
#include <iostream>

bool unpackCommandLineArgs(std::queue<std::string>& args, std::filesystem::path& target,
	std::regex& regexPhrase, std::regex& extensions, bool& verbosity);