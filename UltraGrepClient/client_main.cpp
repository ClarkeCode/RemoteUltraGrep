/*
Created by Robert Clarke and Evan Burgess
Date: 2019-12-08
*/

#include <string>
#include <iostream>
#include <memory>
#include <map>
#include <algorithm>
#include <regex>
#include <mutex>
#include <thread>
#include <queue>
using namespace std;

#include "../UltraGrepServer/WinSockets.hpp"
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

void clientCommunicationHandler(string& clientIp, bool* isProcessingValid, bool* isPendingGrepResults,
	mutex* p_mxInputProcessingQueue, queue<string>& inputProcessingQueue, mutex* p_mxCout) {
	using namespace remote;
	constexpr unsigned short PORT_NUM = 55444;

	map<string, remote::CommandEnum> commands{
		{"grep", remote::GREP},
		{"drop", remote::DROP},
		{"connect", remote::CONNECT},
		{"stopserver", remote::STOPSERVER},
		{"exit", remote::EXIT}
	};

	try {
		shared_ptr<networking::TCPClientSocket> p_clientSock = nullptr;
		std::cout << "Attempting server connection at " << clientIp << endl;

		networking::WindowsSocketActivation wsa;
		p_clientSock = make_shared<networking::TCPClientSocket>(clientIp, PORT_NUM);
		std::cout << "Successfully connected at " << clientIp << "\n\n";
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
					std::cout << line;
				}
				else if (signal == RESPONSETERMINATION) {
					*isPendingGrepResults = false;
					std::cout << endl;
				}
			}

			//Outbound check
			else {
				lock_guard<mutex> lk(*p_mxInputProcessingQueue);
				if (!inputProcessingQueue.empty()) {
					string processLine = inputProcessingQueue.front();
					inputProcessingQueue.pop();

					remote::CommandEnum commIdent = possibleCommands(std::cout, commands, processLine, generateCursor(clientIp).size());
					shared_ptr<remote::RemoteCommand> p_command = nullptr;

					//Disconnect from the server if the user DROP, EXIT, or CONNECTs without dropping first
					if (commIdent == remote::DROP || commIdent == remote::EXIT || (commIdent == remote::CONNECT && p_clientSock != nullptr)) {
						if (p_clientSock == nullptr) continue;
						p_command = make_shared<remote::DropCommand>(processLine);
						p_command->sendCommand(*p_clientSock);
						clientIp = "";
						std::cout << "Disconected from '" << p_clientSock->getIpPortString() << "'\n\n";
						p_clientSock = nullptr;
						if (commIdent == remote::EXIT) { isProcessingValid = false; }
					}

					//Make a new TCPClient connection if the user input a correct ip address
					if (commIdent == remote::CONNECT) {
						p_command = make_shared<remote::ConnectCommand>(processLine);
						if (p_command->isValid) {
							clientIp = p_command->arguments;
							p_clientSock = make_shared<networking::TCPClientSocket>(clientIp, PORT_NUM);
						}
						else {
							std::cout << "'" << p_command->arguments << "' is not a valid IP address\n\n";
						}
					}
					
					//Tell the connected server to shut down
					else if (commIdent == remote::STOPSERVER) {
						if (p_clientSock == nullptr) continue;
						p_command = make_shared<remote::StopServerCommand>(processLine);
						p_command->sendCommand(*p_clientSock);
						clientIp = "";
						std::cout << "Stopped server at '" << p_clientSock->getIpPortString() << "', disconnecting...\n\n";
						p_clientSock = nullptr;
					}

					//Ask the connected server to run the grep command
					else if (commIdent == remote::GREP) {
						if (p_clientSock == nullptr) continue;
						p_command = make_shared<remote::GrepCommand>(processLine);
						p_command->sendCommand(*p_clientSock);

						*isPendingGrepResults = true;
					}
				}
			}

		}
	}
	catch (networking::SocketException & ex) {
		std::cout << ex.what() << endl;
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

	std::cout << "Remote UltraGrep by Robert Clarke and Evan Burgess" << endl;

	try {
		mutex mxInputProcessingQueue;
		mutex mxCout;
		queue<string> inputProcessingQueue;
		thread communicationChannel(
			clientCommunicationHandler, ref(clientIp), &isClientOperational, &isPendingGrepResults, 
			&mxInputProcessingQueue, ref(inputProcessingQueue), &mxCout);

		string line;
		do {
			//If there is an unprocessed command or is receiving a response from the server, block user input
			{
				lock_guard<mutex> lk(mxInputProcessingQueue);
				if (!inputProcessingQueue.empty()) continue;
				if (isPendingGrepResults) continue;
			}

			//Take user input and add it to the processing queue
			{
				lock_guard<mutex> coutlk(mxCout);
				std::cout << generateCursor(clientIp);
				getline(cin, line);
				lock_guard<mutex> lk(mxInputProcessingQueue);
				inputProcessingQueue.push(line);
			}
		} while (isClientOperational);

		std::cout << "Closing client" << endl;
		communicationChannel.join();
		return EXIT_SUCCESS;
	}
	catch (networking::SocketException & ex) {
		std::cout << ex.what() << endl;
	}
}