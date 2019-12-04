/**
@file:		UltraGrep_main.cpp
@author:	Robert Clarke
@date:		2019-11-08
@note:		Developed for the INFO-5104 course at Fanshawe College
@brief:		Defines entry point for the UltraGrep program
*/

#include <string>
#include <iostream>
#include <queue>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <mutex>

#include "CommandLineFunctions.hpp"
#include "ThreadPool.hpp"
#include "FileReporting.hpp"
#include "PerformanceTimer.hpp"

int main(int argc, char* argv[]) {
	using namespace std;
	using namespace filesystem;

	queue<string> cmdLineArgs;
	for (int x = 1; x < argc; x++)
		cmdLineArgs.emplace(argv[x]);

	bool isVerbose = false;
	path targetFolder;
	regex regexString, interestedFileExtensions;

	if (!unpackCommandLineArgs(cmdLineArgs, targetFolder, regexString, interestedFileExtensions, isVerbose))
		return EXIT_FAILURE;

	mutex mxConsoleOutput;
	mutex mxTotalReport;
	vector<FileGrepReport> totalReport;

	//Application specific function pointer to be given to the threadpool
	function<void(path)> longLambda = [&](path p) -> void {
		if (isVerbose) {
			lock_guard<mutex> lk(mxConsoleOutput);
			cout << "Grepping: " << p << endl;
		}
		FileGrepReport fgr = generateFGR(p, regexString, isVerbose, &mxConsoleOutput);
		if (!fgr.empty()) {
			lock_guard<mutex> lk(mxTotalReport);
			totalReport.push_back(fgr);
		}
	};

	timer::PerformanceTimer searchTimer;
	threading::ThreadPool<path> threadPool(longLambda);
	try {
		for (directory_entry const& dirEntry : recursive_directory_iterator(targetFolder)) {
			if (dirEntry.is_directory() && isVerbose) {
				lock_guard<mutex> lk(mxConsoleOutput);
				cout << "Scanning: " << dirEntry.path() << endl;
			}
			if (regex_match(dirEntry.path().extension().string(), interestedFileExtensions)) {
				threadPool.enqueue(dirEntry.path());
			}
		}
	}
	catch (filesystem_error err) {
		cout << "Encountered a filesystem error:\n\t" << err.what() << endl;
	}
	threadPool.wrapup();
	searchTimer.stop();

	cout << endl;
	printGrepReport(totalReport, searchTimer);

	return EXIT_SUCCESS;
}