
#include <string>
#include <iostream>
#include <regex>
#include <memory>
using namespace std;
#include "WinSockets.hpp"
#include "RemoteCommands.hpp"

int main(int argc, char* argv[]) {
	bool isServerOperational = true;
	string serverIp = "127.0.0.1";
	if (argc > 1) {
		if (regex_match(argv[1], regex(R"ipv4format((?:\d{1,3}\.){3}\d{1,3})ipv4format")))
			serverIp = argv[1];
	}

	shared_ptr<networking::TCPClientSocket> p_clientSock = nullptr;
	try {
		networking::WindowsSocketActivation wsa;
		networking::TCPServerSocket serverSock(serverIp, 55444);
		cout << "Server opened at '" << serverSock.getIpPortString() << "'" << endl;

		while (isServerOperational) {
			//Listen for a client connection
			cout << "Waiting for a client to connect..." << endl;
			p_clientSock = serverSock.WaitForConnection();
			cout << "Recieved a client" << endl;

			//While the client socket is pointing to a valid socket object, process the input
			remote::CommandEnum clientInput = remote::NOACTION;
			do {
				p_clientSock->receiveInfo<remote::CommandEnum>(clientInput);
				if (clientInput != remote::NOACTION)
					cout << "Got '" << clientInput << "' from the client" << endl;
				if (clientInput == remote::DROP) {
					cout << "Client disconnected\n\n";
					p_clientSock = nullptr;
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