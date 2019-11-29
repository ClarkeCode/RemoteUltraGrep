
#include <string>
#include <iostream>
using namespace std;
#include "WinSockets.hpp"

int main(int argc, char* argv[]) {
	cout << "We're working" << endl;

	try {
		networking::WindowsSocketActivation wsa;
		networking::TCPServerSocket serverSock("127.0.0.1", 55444);
		networking::TCPClientSocket clientSock = serverSock.WaitForConnection();

		string clientInput;
		do {
			clientSock.receiveInfo<string>(clientInput);
			cout << "Got '" << clientInput << "' from the client" << endl;
		} while (clientInput != "quit");
		return EXIT_SUCCESS;

		{
			//unsigned short xx;
			string yy("Placeholder");

			//clientSock.receiveInfo<unsigned short>(xx);
			clientSock.receiveInfo<string>(yy);
			cout << yy << endl;

			//clientSock.sendInfo<unsigned short>(((unsigned short)12345));
			clientSock.sendInfo<string>("Something from server");

			char ch;
			cout << ">";
			cin >> ch;
		}
	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
	}
}