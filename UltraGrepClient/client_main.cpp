
#include <string>
#include <iostream>
#include <memory> //smart pointers
using namespace std;
#include "../UltraGrepServer/WinSockets.hpp"
#include <map>
#include <algorithm>
#include <regex>
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

string generateCursor(string const& ipAddr) { 
	if (ipAddr != "")
		return "ugrepclient [" + ipAddr + "]> ";
	return "ugrepclient> "; }

string trimString(string const& untrimmed) {
	string leftTrim(untrimmed.begin() + untrimmed.find_first_not_of(' '), untrimmed.end());
	return string(leftTrim.begin(), leftTrim.begin() + leftTrim.find_last_not_of(' ') + 1);
}

int main(int argc, char* argv[]) {
	string clientIp = "127.0.0.1";
	constexpr unsigned short PORT_NUM = 55444;

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

		p_clientSock = make_shared<networking::TCPClientSocket>(clientIp, PORT_NUM);
		cout << "Successfully connected at " << clientIp << "\n\n";

		while (!isClientFinished) {
			string line;
			do {
				cout << generateCursor(clientIp);
				getline(cin, line);
				remote::CommandEnum commIdent = possibleCommands(cout, commands, line, generateCursor(clientIp).size());

				shared_ptr<remote::RemoteCommand> p_command = nullptr;

				switch (commIdent) {
				case remote::DROP:
					p_command = make_shared<remote::DropCommand>(line);
					clientIp = "";
					p_clientSock->sendInfo<remote::CommandEnum>(p_command->_commandType);
					cout << "Disconected from '" << p_clientSock->getIpPortString() << "'\n\n";
					p_clientSock = nullptr;
					break;

				case remote::CONNECT:
					p_command = make_shared<remote::ConnectCommand>(line);
					if (p_command->isValid) {
						clientIp = p_command->arguments;
						p_clientSock = make_shared<networking::TCPClientSocket>(clientIp, PORT_NUM);
					}
					else {
						cout << "'" << p_command->arguments << "' is not a valid IP address\n\n";
					}
					break;

				case remote::STOPSERVER:
					p_command = make_shared<remote::StopServerCommand>(line);

					clientIp = "";
					p_clientSock->sendInfo<remote::CommandEnum>(p_command->_commandType);
					cout << "Stopped server at '" << p_clientSock->getIpPortString() << "', disconnecting...\n\n";
					p_clientSock = nullptr;
					break;

				default:
					break;
				}

				//p_clientSock->sendInfo<string>(line);
			} while (p_clientSock != nullptr);
		}
		

		return EXIT_SUCCESS;
	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
	}
}