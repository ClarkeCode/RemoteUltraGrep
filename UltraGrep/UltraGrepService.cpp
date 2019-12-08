#include "UltraGrepService.hpp"

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <mutex>
#include <functional>

#include "CommandLineFunctions.hpp"
#include "FileReporting.hpp"
#include "PerformanceTimer.hpp"
#include "ThreadPool.hpp"

using namespace std;
using namespace filesystem;

bool UltraGrepService::runUltraGrep(StringQueue& arguments, std::mutex* p_mxOutput, StringQueue& output) {
	 bool isVerbose = false;
	 path targetFolder;
	 regex regexString, interestedFileExtensions;

	 if (!unpackCommandLineArgs(arguments, targetFolder, regexString, interestedFileExtensions, isVerbose, p_mxOutput, output))
		 return false;


	 //mutex mxConsoleOutput;
	 mutex mxTotalReport;
	 vector<FileGrepReport> totalReport;

	 //Application specific function pointer to be given to the threadpool
	 function<void(path)> longLambda = [&](path p) -> void {
		 if (isVerbose) {
			 lock_guard<mutex> lk(*p_mxOutput);
			 output.push("Grepping: " + p.string() + '\n');
		 }
		 FileGrepReport fgr = generateFGR(p, regexString, isVerbose, p_mxOutput, output);
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
				 lock_guard<mutex> lk(*p_mxOutput);
				 output.push("Scanning: " + dirEntry.path().string() + '\n');
			 }
			 if (regex_match(dirEntry.path().extension().string(), interestedFileExtensions)) {
				 threadPool.enqueue(dirEntry.path());
			 }
		 }
	 }
	 catch (filesystem_error err) {
		 output.push("Encountered a filesystem error:\n\t" + string(err.what()) + '\n');
	 }

	 threadPool.wrapup();
	 searchTimer.stop();

	 output.push("\n");
	 printGrepReport(totalReport, searchTimer, p_mxOutput, output);

	 return true;
}