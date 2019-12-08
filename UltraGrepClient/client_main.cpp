
#include <string>
#include <iostream>
#include <memory> //smart pointers
using namespace std;
#include "../UltraGrepServer/WinSockets.hpp"
#include <map>
#include <algorithm>
#include "../UltraGrepServer/RemoteCommands.hpp"

remote::CommandEnum possibleCommands(ostream& os, map<string, remote::CommandEnum>& commands, string& userInput, size_t leftSideOffset = 0) {

	for (map<string, remote::CommandEnum>::iterator it = commands.begin(); it != commands.end(); it++) {
		string testCommand = it->first;
		pair<string::iterator, string::iterator> pair = mismatch(testCommand.begin(), testCommand.end(), userInput.begin(), userInput.end());

		if (pair.first == testCommand.begin()) { //no overlap
			continue;
		}
		else if (pair.first != testCommand.end()) { //mispelling
			string spacing(pair.second - userInput.begin() + leftSideOffset, ' ');
			os << spacing << "^" << endl;
			os << spacing << "Mispelled command! Did you mean: '" << testCommand << "'?" << "\n\n";
			return remote::NOACTION;
		}
		else { //valid command
			os << endl;
			return it->second;
		}
	}
	os << "\n'" << userInput << "' is not a valid command! Commands are:\n";
	for (auto commmandStrings : commands) { os << "    " << commmandStrings.first << "\n"; }
	os << endl;
	return remote::NOACTION;
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

	map<string, remote::CommandEnum> commands{
		{"grep", remote::GREP},
		{"drop", remote::DROP},
		{"connect", remote::CONNECT},
		{"stopserver", remote::STOPSERVER}
		//help - show help + description
		//exit - exit the client
	};

	bool isClientFinished = false;
	shared_ptr<networking::TCPClientSocket> p_clientSock = nullptr;

	cout << "Attempting server connection at " << clientIp << endl;
	try {
		networking::WindowsSocketActivation wsa;

		while (!isClientFinished) {
			p_clientSock = make_shared<networking::TCPClientSocket>(clientIp, 55444);
			cout << "Successfully connected at " << clientIp << "\n\n";

			string line;
			do {
				cout << generateCursor(clientIp);
				getline(cin, line);
				possibleCommands(cout, commands, line, generateCursor(clientIp).size());
				p_clientSock->sendInfo<string>(line);
			} while (p_clientSock != nullptr);
		}
		

		return EXIT_SUCCESS;
	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
	}
}