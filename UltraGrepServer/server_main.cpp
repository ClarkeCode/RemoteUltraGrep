
#include <string>
#include <iostream>
#include <regex>
using namespace std;
#include "WinSockets.hpp"

int main(int argc, char* argv[]) {
	bool isServerOperational = true;
	string serverIp = "127.0.0.1";
	if (argc > 1) {
		if (regex_match(argv[1], regex(R"ipv4format((?:\d{1,3}\.){3}\d{1,3})ipv4format")))
			serverIp = argv[1];
	}

	try {
		networking::WindowsSocketActivation wsa;
		networking::TCPServerSocket serverSock(serverIp, 55444);
		cout << "Server opened at '" << serverSock.getIpPortString() << "'" << endl;
		cout << "Waiting for a client to connect..." << endl;
		networking::TCPClientSocket clientSock = serverSock.WaitForConnection();
		cout << "Recieved a client" << endl;

		string clientInput;
		do {
			clientSock.receiveInfo<string>(clientInput);
			if (clientInput != "")
				cout << "Got '" << clientInput << "' from the client" << endl;
		} while (clientInput != "quit");
		return EXIT_SUCCESS;
	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
		return EXIT_FAILURE;
	}
}