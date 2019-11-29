
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

int main(int argc, char* argv[]) {
	cout << "Working" << endl;

	map<char, string> commands{
		{'g', "grep"},
		{'d', "drop"},
		{'c', "connect"},
		{'s', "stopserver"}
		//help - show help + description
		//kill - exit the client
	};

	string line;
	while (false) {
		string cursor = "ugrepclient> ";
		cout << cursor;
		getline(cin, line);
		possibleCommands(cout, commands, line, cursor.size());
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


		//unsigned short aa = 8989;
		string bb = "Gim";// me something from client";
		//client.sendInfo<unsigned short>(aa);
		client.sendInfo<string>(bb);

		//unsigned short xx;
		string yy("                                        ");
		//client.receiveInfo<unsigned short>(xx);
		client.receiveInfo<string>(yy);
		cout << endl << yy << endl;

		char ch;
		cout << ">";
		cin >> ch;

	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
	}
}