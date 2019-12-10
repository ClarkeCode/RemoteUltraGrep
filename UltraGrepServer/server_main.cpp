/*
Created by Robert Clarke and Evan Burgess 
Date: 2019-12-08
*/
#include <string>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <regex>
#include <memory>
#include <queue>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <functional>
using namespace std;
#include "WinSockets.hpp"
#include "RemoteCommands.hpp"

#include "UltraGrepService.hpp"

template <typename T>
queue<T> queueSplit(string unsplit, char delim = ' ') {
	deque<T> result;
	stringstream ss;
	for (char ch : unsplit) {
		if (ch == delim) {
			result.push_back(ss.str());
			ss.str("");
		}
		else {
			ss << ch;
		}
	}
	result.push_back(ss.str());
	typename deque<T>::iterator it = remove(result.begin(), result.end(), "");
	return queue<T>(result);
}

void outboundChannelProcessing(std::mutex* p_mxOutput, std::queue<std::string>& output, 
	shared_ptr<networking::TCPClientSocket> p_clientSock, bool* isGrepOutputFinished, bool* isOutboundChannelValid) {

	while (*isOutboundChannelValid) {
		if (p_clientSock != nullptr) {
			lock_guard<mutex> lk(*p_mxOutput);
			if (!output.empty()) {
				string transferString = output.front();
				output.pop();
				p_clientSock->sendInfo<remote::CommandEnum>(remote::RESPONSE);
				p_clientSock->sendInfo<string>(transferString);
				cout << "Xfer: " << transferString << endl;
			}
			if (*isGrepOutputFinished) {
				p_clientSock->sendInfo<remote::CommandEnum>(remote::RESPONSETERMINATION);
				*isGrepOutputFinished = false;
				cout << "Xfer finished \n";
			}
		}
	}
}

int main(int argc, char* argv[]) {
	//UltraGrepService ugs;
	bool isServerOperational = true;
	string serverIp = "127.0.0.1";
	if (argc > 1) {
		if (regex_match(argv[1], regex(R"ipv4format((?:\d{1,3}\.){3}\d{1,3})ipv4format")))
			serverIp = argv[1];
	}

	queue<string> outputQueue;
	mutex mxOutputQueue;

	shared_ptr<networking::TCPClientSocket> p_clientSock = nullptr;
	shared_ptr<thread> p_grepThread = nullptr;
	shared_ptr<thread> p_outboundChannel = nullptr;
	bool responseTerminationSignal = false;
	bool isOutboundChannelValid = true;
	try {
		networking::WindowsSocketActivation wsa;
		networking::TCPServerSocket serverSock(serverIp, 55444);
		cout << "Server opened at '" << serverSock.getIpPortString() << "'" << endl;

		while (isServerOperational) {
			//Listen for a client connection
			cout << "Waiting for a client to connect..." << endl;
			p_clientSock = serverSock.WaitForConnection();

			isOutboundChannelValid = true;
			p_outboundChannel = make_shared<thread>(
				outboundChannelProcessing, &mxOutputQueue, ref(outputQueue), p_clientSock, &responseTerminationSignal, &isOutboundChannelValid);
			cout << "Recieved a client" << endl;

			//While the client socket is pointing to a valid socket object, process the input
			remote::CommandEnum clientInput = remote::NOACTION;
			do {
				p_clientSock->receiveInfo<remote::CommandEnum>(clientInput);
				if (clientInput != remote::NOACTION)
					cout << "Got '" << clientInput << "' from the client" << endl;
				if (clientInput == remote::DROP) {
					cout << "Client disconnected\n\n";
					isOutboundChannelValid = false;
					p_outboundChannel->join();
					p_clientSock = nullptr;
				}
				else if (clientInput == remote::STOPSERVER) {
					cout << "Shutting down..." << endl;
					p_clientSock = nullptr;
					isServerOperational = false;
					isOutboundChannelValid = false;
					p_outboundChannel->join();
				}
				else if (clientInput == remote::GREP) {
					string rawArgs;
					p_clientSock->receiveInfo<string>(rawArgs);
					queue<string> splitArgs = queueSplit<string>(rawArgs);

					//function<void(void)> myLambda = [&]() -> void { UltraGrepService::runUltraGrep(splitArgs, &mxOutputQueue, outputQueue); };
					//thread ab(UltraGrepService::runUltraGrep, splitArgs, &mxOutputQueue, outputQueue);
					//thread ab(myLambda);
					//ugs.runUltraGrep(splitArgs, &mxOutputQueue, outputQueue);
					runUltraGrep(splitArgs, &mxOutputQueue, outputQueue);
					responseTerminationSignal = true;
					//p_grepThread = make_shared<thread>(runUltraGrep, ref(splitArgs), &mxOutputQueue, ref(outputQueue));


					//p_clientSock->sendInfo<int>(outputQueue.size());
					//while (!outputQueue.empty()) {
					//	p_clientSock->sendInfo<string>(outputQueue.front());
					//	outputQueue.pop();
					//}
				}
			} while (p_clientSock != nullptr);

		}
		return EXIT_SUCCESS;
	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
		return EXIT_FAILURE;
	}
}