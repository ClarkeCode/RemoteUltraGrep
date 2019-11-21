
#include <string>
#include <iostream>
using namespace std;
#include "../UltraGrepServer/WinSockets.hpp"
#include <map>
#include <algorithm>

void possibleCommands(map<char, string>& commands, string& userInput) {
	map<char, string>::const_iterator it = commands.find(userInput[0]);
	if (it == commands.end())
		cout << "Not a command" << endl;
	else {
		string testCommand = commands[userInput[0]];
		pair<string::iterator, string::iterator> pair = mismatch(testCommand.begin(), testCommand.end(), userInput.begin(), userInput.end());

		if (pair.first != testCommand.end()) {
			cout << string(pair.second - userInput.begin(), ' ') << "^" << endl;
			cout << string(pair.second - userInput.begin(), ' ') << "Mispelled command! Did you mean: '" << testCommand << "'?" << endl;
		}
		else {
			cout << "Good command" << endl;
		}
	}
	cout << endl;
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
		getline(cin, line);
		possibleCommands(commands, line);
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