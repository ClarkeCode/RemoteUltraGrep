
#include <string>
#include <iostream>
using namespace std;
#include "../UltraGrepServer/WinSockets.hpp"
#include <map>
#include <algorithm>

void possibleCommands(ostream& os, map<char, string>& commands, string& userInput, size_t leftSideOffset = 0) {
	map<char, string>::const_iterator it = commands.find(userInput[0]);
	if (it == commands.end())
		os << "Not a command" << endl;
	else {
		string testCommand = commands[userInput[0]];
		pair<string::iterator, string::iterator> pair = mismatch(testCommand.begin(), testCommand.end(), userInput.begin(), userInput.end());

		if (pair.first != testCommand.end()) {
			string spacing(pair.second - userInput.begin() + leftSideOffset, ' ');
			os << spacing << "^" << endl;
			os << spacing << "Mispelled command! Did you mean: '" << testCommand << "'?" << endl;
		}
		else {
			os << "Good command" << endl;
		}
	}
	os << endl;
}

#include <regex>
string generateCursor(string const& ipAddr) { 
	if (ipAddr != "")
		return "ugrepclient [" + ipAddr + "]> ";
	return "ugrepclient> "; }

int main(int argc, char* argv[]) {
	string clientIp = "127.0.0.1";
	if (argc > 1) {
		if (regex_match(argv[1], regex(R"ipv4format((?:\d{1,3}\.){3}\d{1,3})ipv4format"))) {
			clientIp = argv[1];
		}
	}
	cout << "Working" << endl;
	cout << generateCursor(clientIp) << endl;

	map<char, string> commands{
		{'g', "grep"},
		{'d', "drop"},
		{'c', "connect"},
		{'s', "stopserver"}
		//help - show help + description
		//kill - exit the client
	};

	string line;
	while (true) {
		cout << generateCursor(clientIp);
		getline(cin, line);
		possibleCommands(cout, commands, line, generateCursor(clientIp).size());
	}

	try {
		networking::WindowsSocketActivation wsa;
		networking::TCPClientSocket client("127.0.0.1", 55444);

		string comm;
		do {
			cout << ">";
			cin >> comm;
			client.sendInfo<string>(comm);
			cout << "Sent '" << comm << "' to the server\n\n";
		} while (comm != "quit");
		return EXIT_SUCCESS;
	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
	}
}