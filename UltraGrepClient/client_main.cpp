
#include <string>
#include <iostream>
using namespace std;
#include "../UltraGrepServer/WinSockets.hpp"
#include <map>
#include <algorithm>

void possibleCommands(ostream& os, map<char, string>& commands, string& userInput, size_t rightSideOffset = 0) {
	map<char, string>::const_iterator it = commands.find(userInput[0]);
	if (it == commands.end())
		os << "Not a command" << endl;
	else {
		string testCommand = commands[userInput[0]];
		pair<string::iterator, string::iterator> pair = mismatch(testCommand.begin(), testCommand.end(), userInput.begin(), userInput.end());

		if (pair.first != testCommand.end()) {
			string spacing(pair.second - userInput.begin() + rightSideOffset, ' ');
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
	while (true) {
		string cursor = "ugrepclient> ";
		cout << cursor;
		getline(cin, line);
		possibleCommands(cout, commands, line, cursor.size());
	}

	{
		networking::WindowsSocketActivation wsa;
		networking::TCPClientSocket client("127.0.0.1", 55444);

		client.sendInfo<unsigned short>(8989);
		client.sendInfo<string>("Gimme something from client");

		unsigned short xx;
		string yy;
		client.receiveInfo<unsigned short>(xx);
		client.receiveInfo<string>(yy);
		cout << xx << endl << yy << endl;

		char ch;
		cin >> ch;

	}
}