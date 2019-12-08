
#include <string>
#include <iostream>
#include <memory> //smart pointers
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

	map<char, string> commands{
		{'g', "grep"},
		{'d', "drop"},
		{'c', "connect"},
		{'s', "stopserver"}
		//help - show help + description
		//kill - exit the client
	};

	bool applicationFinished = false;
	shared_ptr<networking::TCPClientSocket> p_clientSock = nullptr;

	cout << "Attempting server connection at " << clientIp << endl;
	try {
		networking::WindowsSocketActivation wsa;
		p_clientSock = make_shared<networking::TCPClientSocket>(clientIp, 55444);
		cout << "Successfully connected at " << clientIp << "\n\n";

		string line;
		do {
			cout << generateCursor(clientIp);
			getline(cin, line);
			possibleCommands(cout, commands, line, generateCursor(clientIp).size());
			p_clientSock->sendInfo<string>(line);
		} while (!applicationFinished);

		return EXIT_SUCCESS;
	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
	}
}