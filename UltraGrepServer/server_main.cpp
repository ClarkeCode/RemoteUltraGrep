
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

		{
			unsigned short xx;
			string yy;

			clientSock.receiveInfo<unsigned short>(xx);
			clientSock.receiveInfo<string>(yy);


			clientSock.sendInfo<unsigned short>(((unsigned short)12345));
			clientSock.sendInfo<string>("Something from server");

			char ch;
			cin >> ch;
		}
	}
	catch (networking::SocketException & ex) {
		cout << ex.what() << endl;
	}
}