/**
@file:		CommandLineFunctions.cpp
@author:	Robert Clarke
@date:		2019-11-08
@note:		Developed for the INFO-5104 course at Fanshawe College
@brief:		Implementation of the unpackCommandLineArgs function
*/
#include "CommandLineFunctions.hpp"

bool unpackCommandLineArgs(std::queue<std::string>& args,
	std::filesystem::path& target, std::regex& regexPhrase, std::regex& extensions, bool& verbosity) {
	using namespace std;
	using namespace filesystem;
	if (args.empty()) {
		cout << "Ultragrep implemented by Robert Clarke\nUsage: ultragrep [-v] folder regex [.ext]*" << endl;
		return false;
	}

	//Check if it's in verbose mode 
	string hold = args.front();
	if (hold == "-v") {
		verbosity = true;
		args.pop();
	}

	//Retrieve the target folder
	if (!args.empty())
		hold = args.front();
	else {
		cout << "Missing target folder" << endl;
		return false;
	}
	target = path(hold);
	args.pop();

	//Retrieve the regex phrase
	if (args.empty()) {
		cout << "Missing regex phrase" << endl;
		return false;
	}
	regexPhrase = regex(args.front());
	args.pop();

	//Compose a regex to match the input extensions
	if (args.empty()) {
		extensions = regex("\\.txt");
	}
	else {
		hold = args.front();
		string rep = "|\\.";
		string builder;
		builder.reserve(hold.size() + count(hold.begin(), hold.end(), '.') * rep.size() - 1);
		for (char const& ch : hold) {
			if (ch == '.') builder += rep;
			else builder += ch;
		}
		builder.erase(builder.begin());
		extensions = regex(builder);
	}
	return true;
}
