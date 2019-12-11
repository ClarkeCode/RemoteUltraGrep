/*
Created by Robert Clarke and Evan Burgess
Date: 2019-12-08
*/

#include <string>
#include <iostream>
#include <memory> //smart pointers
using namespace std;
#include "../UltraGrepServer/WinSockets.hpp"
#include <map>
#include <algorithm>
#include <regex>
#include "../UltraGrepServer/RemoteCommands.hpp"
#include <mutex>
#include <thread>
#include <queue>

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
	for (auto commmandStrings : commands) { 
		os << "    " << commmandStrings.first << (commmandStrings.first == "connect" ? " address" : "") << 
			(commmandStrings.first == "grep" ? " [-v] remotefolder regex [.ext]*" : "") << "\n";
	}
	os << endl;
	return remote::NOACTION;
}

string generateCursor(string const& ipAddr) { 
	if (ipAddr != "")
		return "ugrepclient [" + ipAddr + "]> ";
	return "ugrepclient> ";
}


void processInboundChannel(bool* isThreadValid, bool* isPendingGrepResults, 
	mutex* p_mxClientSocket, shared_ptr<networking::TCPClientSocket> p_clientSock) {
	using namespace remote;
	while (*isThreadValid) {
		if (p_clientSock != nullptr && *isPendingGrepResults) {
			lock_guard<mutex> lk(*p_mxClientSocket);
			CommandEnum signal = NOACTION;
			p_clientSock->receiveInfo<CommandEnum>(signal);
			if (signal == NOACTION) continue;

			if (signal == RESPONSE) {
				string line;
				p_clientSock->receiveInfo<string>(line);
				cout << line;
			}
			else if (signal == RESPONSETERMINATION) {
				*isPendingGrepResults = false;
				cout << endl;
			}
		}
	}
}

void clientCommunicationHandler(string& clientIp, bool* isProcessingValid, bool* isPendingGrepResults,
	mutex* p_mxInputProcessingQueue, queue<string>& inputProcessingQueue, mutex* p_mxCout) {
	using namespace remote;
	constexpr unsigned short PORT_NUM = 55444;

	map<string, remote::CommandEnum> commands{
		{"grep", remote::GREP},
		{"drop", remote::DROP},
		{"connect", remote::CONNECT},
		{"stopserver", remote::STOPSERVER}
		//help - show help + description
		//exit - exit the client
	};

	try {
		shared_ptr<networking::TCPClientSocket> p_clientSock = nullptr;
		cout << "Attempting server connection at " << clientIp << endl;

		networking::WindowsSocketActivation wsa;
		p_clientSock = make_shared<networking::TCPClientSocket>(clientIp, PORT_NUM);
		cout << "Successfully connected at " << clientIp << "\n\n";
		*isPendingGrepResults = false;

		while (*isProcessingValid) {
			lock_guard<mutex> coutlk(*p_mxCout);

			//See if the inbound channel should be checked
			if (*isPendingGrepResults) {
				CommandEnum signal = NOACTION;
				p_clientSock->receiveInfo<CommandEnum>(signal);
				if (signal == NOACTION) continue;

				if (signal == RESPONSE) {
					string line;
					p_clientSock->receiveInfo<string>(line);
					cout << line;
				}
				else if (signal == RESPONSETERMINATION) {
					*isPendingGrepResults = false;
					cout << endl;
				}
			}

			//Outbound check
			else {
				lock_guard<mutex> lk(*p_mxInputProcessingQueue);
				if (!inputProcessingQueue.empty()) {
					string processLine = inputProcessingQueue.front();
					inputProcessingQueue.pop();

					remote::CommandEnum commIdent = possibleCommands(cout, commands, processLine, generateCursor(clientIp).size());
					shared_ptr<remote::RemoteCommand> p_command = nullptr;

					if (commIdent == remote::DROP || (commIdent == remote::CONNECT && p_clientSock != nullptr)) {
						if (p_clientSock == nullptr) continue;
						p_command = make_shared<remote::DropCommand>(processLine);
						p_command->sendCommand(*p_clientSock);
						clientIp = "";
						cout << "Disconected from '" << p_clientSock->getIpPortString() << "'\n\n";
						p_clientSock = nullptr;
					}
					if (commIdent == remote::CONNECT) {
						p_command = make_shared<remote::ConnectCommand>(processLine);
						if (p_command->isValid) {
							clientIp = p_command->arguments;
							p_clientSock = make_shared<networking::TCPClientSocket>(clientIp, PORT_NUM);
						}
						else {
							cout << "'" << p_command->arguments << "' is not a valid IP address\n\n";
						}
					}
					else if (commIdent == remote::STOPSERVER) {
						if (p_clientSock == nullptr) continue;
						p_command = make_shared<remote::StopServerCommand>(processLine);
						p_command->sendCommand(*p_clientSock);
						clientIp = "";
						cout << "Stopped server at '" << p_clientSock->getIpPortString() << "', disconnecting...\n\n";
						p_clientSock = nullptr;
					}
					else if (commIdent == remote::GREP) {
						p_command = make_shared<remote::GrepCommand>(processLine);
						p_command->sendCommand(*p_clientSock);

						*isPendingGrepResults = true;
					}
				}
			}

		}
	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
	}
	isProcessingValid = false;
}


int main(int argc, char* argv[]) {
	string clientIp = "127.0.0.1";
	constexpr unsigned short PORT_NUM = 55444;

	if (argc > 1) {
		if (regex_match(argv[1], regex(R"ipv4format((?:\d{1,3}\.){3}\d{1,3})ipv4format"))) {
			clientIp = argv[1];
		}
	}


	bool isClientOperational = true;
	bool isPendingGrepResults = true;

	//shared_ptr<thread> p_communicationChannel = nullptr;

	try {
		mutex mxInputProcessingQueue;
		mutex mxCout;
		queue<string> inputProcessingQueue;
		thread communicationChannel(
			clientCommunicationHandler, ref(clientIp), &isClientOperational, &isPendingGrepResults, 
			&mxInputProcessingQueue, ref(inputProcessingQueue), &mxCout);

		string line;
		do {
			{
				lock_guard<mutex> lk(mxInputProcessingQueue);
				if (!inputProcessingQueue.empty()) continue;
				if (isPendingGrepResults) continue;
			}

			{
				lock_guard<mutex> coutlk(mxCout);
				cout << generateCursor(clientIp);
				getline(cin, line);
				lock_guard<mutex> lk(mxInputProcessingQueue);
				inputProcessingQueue.push(line);
			}

		} while (isClientOperational);
		communicationChannel.join();

		return EXIT_SUCCESS;
	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
	}
}