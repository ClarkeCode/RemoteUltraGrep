/**
@file:		FileReporting.hpp
@author:	Robert Clarke
@date:		2019-11-08
@note:		Developed for the INFO-5104 course at Fanshawe College
@brief:		Declaration of the FileGrepReport struct and related functions
*/
#pragma once
#include "PerformanceTimer.hpp"
#include <fstream>
#include <string>
#include <queue>
#include <sstream>

struct FileGrepReport {
	std::filesystem::path filePath;
	using GrepReportLine = std::tuple<size_t, size_t, std::string>;
	using StringQueue = std::queue<std::string>;
	std::vector<GrepReportLine> matchRecords;

	FileGrepReport(std::filesystem::path const& p = std::filesystem::path()) : filePath(p) {}
	inline bool empty() { return matchRecords.empty(); }
	inline size_t size() { return matchRecords.size(); }
};

inline FileGrepReport generateFGR(std::filesystem::path const& targetPath, std::regex const& phrase, 
	bool const& isVerbose, std::mutex* p_mxOutput, FileGrepReport::StringQueue output) {
	using namespace std;
	using namespace filesystem;
	FileGrepReport fileSummary(targetPath);
	ifstream targetFile(targetPath.string());

	if (targetFile.is_open()) {
		string line;
		size_t lineNumber = 1;
		stringstream os;
		while (getline(targetFile, line)) {
			sregex_iterator matchIterator = sregex_iterator(line.begin(), line.end(), phrase);
			sregex_iterator endIterator;
			size_t matchAmount = distance(matchIterator, endIterator);

			if (matchAmount != 0) {
				if (isVerbose) {
					lock_guard<mutex> lk(*p_mxOutput);
					os << "Matched " << matchAmount << ": " << fileSummary.filePath << " [" << lineNumber << "] ";
					os << line << endl;
					output.push(os.str());
					os.str("");
					os.clear();
				}
				fileSummary.matchRecords.push_back(FileGrepReport::GrepReportLine(lineNumber, matchAmount, line));
			}

			lineNumber++;
		}
		targetFile.close();
	}
	return fileSummary;
}


inline void printGrepReport(std::vector<FileGrepReport> const& totalReport, timer::PerformanceTimer& timer,
	std::mutex* p_mxOutput, FileGrepReport::StringQueue output) {
	std::stringstream os;
	os << "Grep Report:\n\n";
	size_t totalMatches = 0;

	for (FileGrepReport const& fgr : totalReport) {
		os << fgr.filePath << std::endl;
		for (FileGrepReport::GrepReportLine const& reportLine : fgr.matchRecords) {
			totalMatches += std::get<1>(reportLine);
			os << "[" << std::get<0>(reportLine);
			if (std::get<1>(reportLine) != 1)
				os << ':' << std::get<1>(reportLine);
			os << "] " << std::get<2>(reportLine) << std::endl;
		}
		os << std::endl;
	}

	os << "Files with matches = " << totalReport.size() << std::endl;
	os << "Total number of matches = " << totalMatches << std::endl;
	os << "Scan completed in " << timer.getTime() << "s" << std::endl;

	{
		std::lock_guard<std::mutex> lk(*p_mxOutput);
		output.push(os.str());
	}
}