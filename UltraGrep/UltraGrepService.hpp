#pragma once
#include <string>
#include <iostream>
#include <queue>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <mutex>
#include <functional>

#include "CommandLineFunctions.hpp"
#include "FileReporting.hpp"
#include "PerformanceTimer.hpp"
#include "ThreadPool.hpp"

struct UltraGrepService {
	using StringQueue = std::queue<std::string>;
	static bool runUltraGrep(StringQueue arguments, std::ostream& os) {
		using namespace std;
		using namespace filesystem;

		bool isVerbose = false;
		path targetFolder;
		regex regexString, interestedFileExtensions;

		if (!unpackCommandLineArgs(arguments, targetFolder, regexString, interestedFileExtensions, isVerbose))
			return false;


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

		return true;
	}
};